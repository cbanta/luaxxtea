package = "xxtea"
version = "1.0-1"
source = { url = "http://github.com/downloads/cbanta/luaxxtea/xxtea-1.0.tar.gz" }
description={
   summary = 'xxtea encryption for lua',
   detailed = 'xxtea encryption for lua',
   homepage = "http://github.com/cbanta/luaxxtea",
   license = "MIT"
}
dependencies = { "lua >= 5.1" }
build = {
	type='builtin',
	modules={
		xxtea='xxtea.c'
	}
}

