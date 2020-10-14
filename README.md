# To3mx

- Convert OSGB PagedLOD tree to Bentley ContextCapture [3MX/3MXB](https://docs.bentley.com/LiveContent/web/ContextCapture%20Help-v9/en/GUID-CED0ABE6-2EE3-458D-9810-D87EC3C521BD.html) tree.

This project has moved to [lodToolkit](https://github.com/ProjSEED/lodToolkit)

### How to use
```
To3mx.exe --input <DIR> --output <DIR>
	-i, --input <DIR> 
	-o, --output <DIR> 
```

### Example
```
To3mx.exe -i E:\Data\Test -o E:\Data\Test_3mx
```

### The input dir should look like this
```
--metadata.xml

--Data\Tile_000_000\Tile_000_000.osgb

```
