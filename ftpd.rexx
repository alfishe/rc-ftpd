
cmd:		VERSION
result:	<ver>.<rev>

cmd:		FTPD CMD/A,ARGS/F

cmds:		FTPD OPEN <0/1>
result:	<0/1>
cmds:		FTPD ANON <0/1>
result:	<0/1>
cmds:		FTPD MAXCPS <value>
result:	<oldvalue>




cmd:		FTP CMD/A,HANDLE/N,STEM/K,ARGS/F

cmds:		FTP KICK <handle>
cmds:		FTP ABORT <handle>
cmds:		FTP LOG <handle> [0-4]
cmds:		FTP MESSAGE <handle> [text]

cmds:		FTP QUERY ALL STEM <stem>
cmds:		FTP QUERY XFER STEM <stem>
cmds:		FTP QUERY <handle> STEM <stem>
stem:		stem.user = <user handle>
			stem.pass = <email>
			stem.ip   = <0.0.0.0>
			stem.file = <filename>
			stem.cps  = <cps>
			stem.size = <filesize>
			stem.done = <current pos>
			stem.path = <alias path>
			stem.real = <real path>
			stem.access = <access handle>
			stem.log  = <loglevel>




cmd:		USER CMD/A,HANDLE/N/A,STEM/K,ARGS/F

cmds:		USER QUERY ALL <stem>

cmds:		USER QUERY <handle> <stem>
cmds:		USER SET <handle> <stem>
cmds:		USER ADD <handle> <stem>
stem:		stem.name = <user name>
			stem.pass = <user pass>
			stem.groups = <user groups>
			stem.max = <max login>
			stem.log = [0-4]




cmd:		GROUP CMD/A,HANDLE/N/A,STEM/K,ARGS/F

cmds:		GROUP QUERY ALL <stem>

cmds:		GROUP QUERY <handle> <stem>
cmds:		GROUP SET <handle> <stem>
cmds:		GROUP ADD <handle> <stem>
stem:		stem.alias = <alias for ftp>
			stem.path = <real path, "" means virtual>
			stem.read = <read access>>
			stem.write = <write access>
			stem.delete = <delete access>
			stem.subdirs = <subdirs access>


