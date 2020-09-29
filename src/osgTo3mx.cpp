#include "osgTo3mx.h"

namespace seed
{
	namespace io
	{
		bool OsgTo3mx::Convert(const std::string& input, const std::string& output)
		{
			std::string inputMetadata = input + "/metadata.xml";
			std::string inputData = input + "/Data/";
			
			std::string outputMetadata = output + "/metadata.xml";
			std::string output3mx = output + "/Root.3mx";
			std::string outputData = output + "/Data/";
			std::string outputDataRootRelative = "Data/Root.3mxb";
			std::string outputDataRoot = output + "/" + outputDataRootRelative;

			if (!utils::CheckOrCreateFolder(output))
			{
				seed::log::DumpLog(seed::log::Critical, "Create folder %s failed!", output.c_str());
				return false;
			}
			if (!utils::CheckOrCreateFolder(outputData))
			{
				seed::log::DumpLog(seed::log::Critical, "Create folder %s failed!", outputData.c_str());
				return false;
			}

			if (!GenerateMetadata(outputMetadata))
			{
				seed::log::DumpLog(seed::log::Critical, "Generate %s failed!", outputMetadata.c_str());
				return false;
			}

			if (!ConvertMetadataTo3mx(inputMetadata, outputDataRootRelative, output3mx))
			{
				seed::log::DumpLog(seed::log::Critical, "Generate %s failed!", output3mx.c_str());
				return false;
			}

			std::vector<Node> nodes;
			std::vector<Resource> resources;
			osgDB::DirectoryContents dirNames = osgDB::getDirectoryContents(inputData);

			// TODO: show progress, support multi-thread 
			for each (std::string dir in dirNames)
			{
				if (dir.find(".") != std::string::npos)
					continue;
				std::string output3mxbName = dir + "/" + dir + ".3mxb";
				std::string output3mxb = outputData + output3mxbName;
				osg::BoundingBox bb;
				if (!ConvertTile(inputData, outputData, dir, bb))
				{
					seed::log::DumpLog(seed::log::Critical, "Generate %s failed!", output3mxb.c_str());
					return false;
				}
				Node node;
				node.id = dir;
				node.bb = bb;
				node.maxScreenDiameter = 0;
				node.children.push_back(output3mxbName);
				nodes.emplace_back(node);
			}

			if (!Generate3mxb(nodes, resources, outputDataRoot))
			{
				seed::log::DumpLog(seed::log::Critical, "Generate %s failed!", outputDataRoot.c_str());
				return false;
			}

			return true;
		}

		bool OsgTo3mx::GenerateMetadata(const std::string& output)
		{
			std::ofstream outfile(output);
			if (outfile.bad())
			{
				seed::log::DumpLog(seed::log::Critical, "Can NOT open file %s!", output.c_str());
				return false;
			}
			outfile << "<?xml version=\"1.0\" encoding=\"utf - 8\"?>\n";
			outfile << "<ModelMetadata version=\"1\">\n";
			outfile << "	<Texture>\n";
			outfile << "		<ColorSource>Visible</ColorSource>\n";
			outfile << "	</Texture>\n";
			outfile << "</ModelMetadata>\n";
			return true;
		}

		bool OsgTo3mx::ConvertMetadataTo3mx(const std::string& input, const std::string& outputDataRootRelative, const std::string& output)
		{
			std::string srs;
			osg::Vec3d srsOrigin(0, 0, 0);
			auto xml = osgDB::readXmlFile(input);
			if (xml)
			{
				for (auto i : xml->children)
				{
					if (i->name == "ModelMetadata")
					{
						for (auto j : i->children)
						{
							if (j->name == "SRS")
							{
								srs = j->contents;
							}
							if (j->name == "SRSOrigin")
							{
								sscanf(j->contents.c_str(), "%lf,%lf,%lf", &srsOrigin._v[0], &srsOrigin._v[1], &srsOrigin._v[2]);
							}
						}
						break;
					}
				}
			}
			else
			{
				seed::log::DumpLog(seed::log::Warning, "Can NOT open file %s!", input.c_str());
			}

			neb::CJsonObject oJson;
			oJson.Add("3mxVersion", 1);
			oJson.Add("name", "Root");
			oJson.Add("description", "Converted by ProjSEED/To3mx, copyright <a href='https://github.com/ProjSEED/To3mx' target='_blank'>ProjSEED</a>.");
			oJson.Add("logo", "logo.png");

			neb::CJsonObject oJsonSceneOption;
			oJsonSceneOption.Add("navigationMode", "ORBIT");
			oJson.AddEmptySubArray("sceneOptions");
			oJson["sceneOptions"].Add(oJsonSceneOption);

			neb::CJsonObject oJsonLayer;
			oJsonLayer.Add("type", "meshPyramid");
			oJsonLayer.Add("id", "mesh0");
			oJsonLayer.Add("name", "Root");
			oJsonLayer.Add("description", "Converted by ProjSEED/To3mx, copyright <a href='https://github.com/ProjSEED/To3mx' target='_blank'>ProjSEED</a>.");
			oJsonLayer.Add("SRS", srs);
			oJsonLayer.AddEmptySubArray("SRSOrigin");
			oJsonLayer["SRSOrigin"].Add(srsOrigin.x());
			oJsonLayer["SRSOrigin"].Add(srsOrigin.y());
			oJsonLayer["SRSOrigin"].Add(srsOrigin.z());
			oJsonLayer.Add("root", outputDataRootRelative);
			oJson.AddEmptySubArray("layers");
			oJson["layers"].Add(oJsonLayer);

			std::ofstream outfile(output);
			if (outfile.bad())
			{
				seed::log::DumpLog(seed::log::Critical, "Can NOT open file %s!", output.c_str());
				return false;
			}
			outfile << oJson.ToFormattedString();;
			return true;
		}

		bool OsgTo3mx::ConvertTile(const std::string& inputData, const std::string& outputData, const std::string& tileName, osg::BoundingBox& bb)
		{
			std::string inputTile = inputData + tileName + "/";
			std::string outputTile = outputData + tileName + "/";
			if (!utils::CheckOrCreateFolder(outputTile))
			{
				seed::log::DumpLog(seed::log::Critical, "Create folder %s failed!", outputTile.c_str());
				return false;
			}
			// top level
			{
				std::string inputOsgb = inputTile + tileName + ".osgb";
				std::string output3mxb = outputTile + tileName + ".3mxb";
				if (!ConvertOsgbTo3mxb(inputOsgb, output3mxb, &bb))
				{
					seed::log::DumpLog(seed::log::Critical, "Convert %s failed!", inputOsgb.c_str());
					return false;
				}
			}
			// all other
			osgDB::DirectoryContents fileNames = osgDB::getDirectoryContents(inputTile);
			for each (std::string file in fileNames)
			{
				std::string ext = osgDB::getLowerCaseFileExtension(file);
				if (ext != "osgb")
					continue;

				std::string baseName = osgDB::getNameLessExtension(file);
				if (baseName == tileName)
					continue;

				std::string inputOsgb = inputTile + baseName + ".osgb";
				std::string output3mxb = outputTile + baseName + ".3mxb";

				if (!ConvertOsgbTo3mxb(inputOsgb, output3mxb))
				{
					seed::log::DumpLog(seed::log::Critical, "Convert %s failed!", inputOsgb.c_str());
					return false;
				}
			}

			return true;
		}

		void OsgTo3mx::ParsePagedLOD(osg::PagedLOD* lod, int index, Node& node, Resource& resGeometry, Resource& resTexture)
		{
			osg::BoundingBox bb;
			bb.expandBy(lod->getBound());

			node.id = "node" + std::to_string(index);
			node.bb = bb;
			node.maxScreenDiameter = lod->getRangeList().size() >= 2 ? lod->getRangeList()[1].first : 0;
			if (lod->getNumFileNames() >= 2)
			{
				std::string baseName = osgDB::getNameLessExtension(lod->getFileName(1));
				node.children.push_back(baseName + ".3mxb");
			}
			node.resources.push_back("geometry" + std::to_string(index));
			
			if (!lod->getNumChildren())
			{
				return;
			}
			if (lod->getNumChildren() > 1)
			{
				seed::log::DumpLog(seed::log::Warning, "PagedLOD has more than 1 Geode!");
			}
			osg::Geode* geode = lod->getChild(0)->asGeode();
			if (!geode)
			{
				return;
			}
			ParseGeode(geode, index, resGeometry, resTexture);
		}

		void OsgTo3mx::ParseGeode(osg::Geode* geode, int index, Resource& resGeometry, Resource& resTexture)
		{
			osg::BoundingBox bb;
			bb.expandBy(geode->getBound());

			resTexture.type = "textureBuffer";
			resTexture.format = "jpg";
			resTexture.id = "texture" + std::to_string(index);
			//resTexture.bufferData; // TODO

			resGeometry.type = "geometryBuffer";
			resGeometry.format = "ctm";
			resGeometry.id = "geometry" + std::to_string(index);
			resGeometry.texture = "texture" + std::to_string(index);
			resGeometry.bb = bb;
			//resGeometry.bufferData; // TODO
		}

		bool OsgTo3mx::ConvertOsgbTo3mxb(const std::string& input, const std::string& output, osg::BoundingBox* pbb)
		{
			//seed::log::DumpLog(seed::log::Debug, "Convert %s ...", input.c_str());
			std::vector<Node> nodes;
			std::vector<Resource> resources;
			osg::ref_ptr<osg::Node> osgNode = osgDB::readNodeFile(input);
			if (dynamic_cast<osg::PagedLOD*>(osgNode.get()))
			{
				// 1 node (1 child, 1 geometryBuffer, 1 textureBuffer)
				int i = 0;
				osg::PagedLOD* lod = dynamic_cast<osg::PagedLOD*>(osgNode.get());

				Node node;
				Resource resGeometry;
				Resource resTexture;

				ParsePagedLOD(lod, i, node, resGeometry, resTexture);

				nodes.emplace_back(node);
				resources.emplace_back(resTexture);
				resources.emplace_back(resGeometry);
			}
			else if (osgNode->asGroup())
			{
				// 4 node (1 child, 1 geometryBuffer, 1 textureBuffer)
				osg::Group* group = osgNode->asGroup();
				for (int i = 0; i < group->getNumChildren(); ++i)
				{
					if (dynamic_cast<osg::PagedLOD*>(group->getChild(i)))
					{
						osg::PagedLOD* lod = dynamic_cast<osg::PagedLOD*>(group->getChild(i));

						Node node;
						Resource resGeometry;
						Resource resTexture;

						ParsePagedLOD(lod, i, node, resGeometry, resTexture);

						nodes.emplace_back(node);
						resources.emplace_back(resGeometry);
						resources.emplace_back(resTexture);
					}
				}
			}
			else if (osgNode->asGeode())
			{
				// 1 node (0 child, 1 geometryBuffer, 1 textureBuffer)
				int i = 0;
				osg::Geode* geode = osgNode->asGeode();
				osg::BoundingBox bb;
				bb.expandBy(geode->getBound());

				Node node;
				Resource resGeometry;
				Resource resTexture;

				node.id = "node" + std::to_string(i);
				node.bb = bb;
				node.maxScreenDiameter = 1e30;

				ParseGeode(geode, i, resGeometry, resTexture);

				nodes.emplace_back(node);
				resources.emplace_back(resGeometry);
				resources.emplace_back(resTexture);
			}
			else
			{
				return false;
			}

			if(!Generate3mxb(nodes, resources, output))
			{
				return false;
			}

			return true;
		}

		bool OsgTo3mx::Generate3mxb(const std::vector<Node>& nodes, const std::vector<Resource>& resources, const std::string& output)
		{
			neb::CJsonObject oJson;
			oJson.Add("version", 1);

			oJson.AddEmptySubArray("nodes");
			for (const auto& node : nodes)
			{
				oJson["nodes"].Add(NodeToJson(node));
			}

			oJson.AddEmptySubArray("resources");
			for (const auto& resource : resources)
			{
				oJson["resources"].Add(ResourceToJson(resource));
			}

			std::string jsonStr = oJson.ToString();
			uint32_t length = jsonStr.size();

			std::ofstream outfile(output, std::ios::out | std::ios::binary);
			outfile.write("3MXBO", 5);
			outfile.write((char*)&length, 4);
			outfile.write(jsonStr.c_str(), length);

			for (const auto& resource : resources)
			{
				outfile.write(resource.bufferData.data(), resource.bufferData.size());
			}

			return true;
		}

		neb::CJsonObject OsgTo3mx::NodeToJson(const Node& node)
		{
			neb::CJsonObject oJson;
			oJson.Add("id", node.id);

			oJson.AddEmptySubArray("bbMin");
			oJson["bbMin"].Add(node.bb.xMin());
			oJson["bbMin"].Add(node.bb.yMin());
			oJson["bbMin"].Add(node.bb.zMin());

			oJson.AddEmptySubArray("bbMax");
			oJson["bbMax"].Add(node.bb.xMax());
			oJson["bbMax"].Add(node.bb.yMax());
			oJson["bbMax"].Add(node.bb.zMax());

			oJson.Add("maxScreenDiameter", node.maxScreenDiameter);

			oJson.AddEmptySubArray("children");
			for (auto child : node.children)
			{
				oJson["children"].Add(child);
			}

			oJson.AddEmptySubArray("resources");
			for (auto resource : node.resources)
			{
				oJson["resources"].Add(resource);
			}
			return oJson;
		}

		neb::CJsonObject OsgTo3mx::ResourceToJson(const Resource& resource)
		{
			neb::CJsonObject oJson;
			oJson.Add("type", resource.type);
			oJson.Add("format", resource.format);
			oJson.Add("id", resource.id);
			if (resource.type == "geometryBuffer")
			{
				oJson.Add("texture", resource.texture);

				oJson.AddEmptySubArray("bbMin");
				oJson["bbMin"].Add(resource.bb.xMin());
				oJson["bbMin"].Add(resource.bb.yMin());
				oJson["bbMin"].Add(resource.bb.zMin());

				oJson.AddEmptySubArray("bbMax");
				oJson["bbMax"].Add(resource.bb.xMax());
				oJson["bbMax"].Add(resource.bb.yMax());
				oJson["bbMax"].Add(resource.bb.zMax());
			}
			oJson.Add("size", resource.bufferData.size());
			return oJson;
		}

	}
}