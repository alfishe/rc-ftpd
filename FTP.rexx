/* SnorsleX FTP Client v1.1 (07/03/2000) ©1999-2000 Lorens Johansson <SnorsleX@Home.se> */

/* History (not complete):
20000307:
· Added so it prints how much you have dled (not for resume, and it doesn't
  print the total).

*/

/* FTP RFC = 765 */

OPTIONS RESULTS

date="07/03/2000"
ver="1.1"
connected=0
download_to=PRAGMA("D")
CR='0d'x

IF RIGHT(download_to,1)~=":"&RIGHT(download_to,1)~="/" THEN download_to=download_to"/"

CALL ADDLIB("rxsocket.library",0,-30,0)
CALL ADDLIB("rxlibnet.library",0,-30,0)
CALL ADDLIB("rexxsupport.library",0,-30,0)

SAY "SnorsleX FTP Client v"ver" ("date")"

DO FOREVER
	ADDRESS COMMAND "echo noline SnorsleXFTP: "
	PARSE PULL command" "options

	SELECT
	WHEN UPPER(command)="?" THEN CALL Main_?
	WHEN UPPER(command)="EXIT" THEN CALL Main_Exit

	WHEN UPPER(command)="CD"   THEN CALL Ftp_CWD(options)
	WHEN UPPER(command)="CDUP" THEN CALL Ftp_CDUP
	WHEN UPPER(command)="DELE" THEN CALL Ftp_DELE(options)
	WHEN UPPER(command)="GET"  THEN CALL Ftp_RETR(options)
	WHEN UPPER(command)="HELP" THEN CALL Ftp_HELP(options)
	WHEN UPPER(command)="LCD"  THEN CALL Ftp_LCD(options)
	WHEN UPPER(command)="LS"   THEN CALL Ftp_LIST(options)
	WHEN UPPER(command)="EPLF" THEN CALL Ftp_EPLF(options)
	WHEN UPPER(command)="FEAT" THEN CALL Ftp_FEAT
	WHEN UPPER(command)="MKD"  THEN CALL Ftp_MKD(options)
	WHEN UPPER(command)="NOOP" THEN CALL Ftp_NOOP
	WHEN UPPER(command)="OPEN" THEN CALL Ftp_CONN(options)
	WHEN UPPER(command)="PASV" THEN CALL Ftp_PASV
	WHEN UPPER(command)="PORT" THEN CALL Ftp_PORT
	WHEN UPPER(command)="PWD"  THEN CALL Ftp_PWD
	WHEN UPPER(command)="RAW"  THEN CALL Ftp_RAW(options)
	WHEN UPPER(command)="RENA" THEN CALL Ftp_RENA(options)
	WHEN UPPER(command)="REST" THEN CALL Ftp_REST(options)
	WHEN UPPER(command)="RMD"  THEN CALL Ftp_RMD(options)
	WHEN UPPER(command)="PUT"  THEN CALL Ftp_STOR(options)
	WHEN UPPER(command)="SYST" THEN CALL Ftp_SYST
	WHEN UPPER(command)="TYPE" THEN CALL Ftp_TYPE(options)
	WHEN UPPER(command)="QUIT" THEN CALL Ftp_QUIT
		OTHERWISE SAY "Unknown command, type ? for help!"
	END
/*
*USER   ACCT  *PASS  *TYPE  *LIST  *CWD
*DELE   NAME  *QUIT  *RETR  *STOR   PORT
 NLST  *PWD    XPWD  *MKD    XMKD   XRMD
*RMD    STRU   MODE  *PASV
*/

/* ADD MODE C (compression)
save to disk <file>.Z
run compress on it.
*/

/* I WILL add the following:
         APPEND (with create) (APPE)

            This command causes the server-DTP to accept the data
            transferred via the data connection and to store the data in
            a file at the server site.  If the file specified in the
            pathname exists at the server site, then the data shall be
            appended to that file; otherwise the file specified in the
            pathname shall be created at the server site.
*/

END

Main_?:
	SAY "Help section of SnorsleX FTP Client v"ver" ("date")"
	SAY "-----------------------------------------------------"
	SAY "FTP Commands:"
	SAY "   CD   [dir]"
	SAY "   CDUP"
	SAY "   DELE <file>"
	SAY "   GET  <file>"
	SAY "   HELP [cmd]"
	SAY "   LCD  [path]"
	SAY "   LS   [args]"
	SAY "   MKD  <dir>"
/*         MDTM <dir>     <-- What?? Direct after MKD */
	SAY "   NOOP"
	SAY "   OPEN [host[/port]]"
	SAY "   PASV"
	SAY "   PORT"
	SAY "   PUT  <local_path_and_file>"
	SAY "   PWD"
	SAY "   QUIT"
	SAY "   RAW <cmd> [args]"
	SAY "   RENA <old_name> <new_name>     It's RFC compliant!"
	SAY "   REST <size>"
	SAY "   RMD  <dir>"
	SAY "   SYST"
	SAY "   TYPE [I/B/A]"
	SAY
	SAY "Other Commands:"
	SAY "   ?    - This helpscreen."
	SAY "   EXIT - Quits the Client."
RETURN

Main_Exit:
	IF connected=1 THEN CALL Ftp_QUIT
EXIT

Ftp_CDUP:
	CALL WRITELN(1,"CDUP")
	CALL Ftp_ReadLines
RETURN

Ftp_CONN:
	conn_args=ARG(1)
	IF connected=1 THEN DO
		SAY "Already connected, QUIT first"
		RETURN
	END

	IF conn_args="" THEN DO
		ADDRESS COMMAND "echo noline Connect to: "
		PARSE PULL hostline
	END
	ELSE hostline=conn_args

	IF POS("/",hostline)>0 THEN DO
		PARSE VAR hostline host '/' port
	END
	ELSE DO
		host=hostline
		port=21
	END

	IF host=""|DATATYPE(port,"W")=0 THEN DO
		SAY "You must enter the host and port!"
		RETURN
	END

	IF ~OPEN(1,"tcp:"host"/"port,"RW") THEN DO
		SAY "Couldn't connect to "host"/"port"..."
		RETURN
	END
	SAY "Connected to "host"/"port"..."
	connected=1
	CALL Ftp_ReadLines

	ADDRESS COMMAND "echo noline User: "
	PARSE PULL user

	CALL WRITELN(1,"USER "user)
	CALL Ftp_ReadLines

	pass=GetPass("Password: ") /* rxlibnet.library */

	CALL WRITELN(1,"PASS "pass)
	CALL Ftp_ReadLines
	IF LEFT(result,3)>499 THEN DO
		CALL Ftp_QUIT
		RETURN
	END

	CALL Ftp_SYST
	CALL Ftp_TYPE("I")
	CALL Ftp_CWD("/")
RETURN

Ftp_CWD:
	IF ARG(1)="" THEN DO
		CALL Ftp_PWD
		RETURN
	END
	ELSE CALL WRITELN(1,"CWD "ARG(1))
	CALL Ftp_ReadLines
RETURN

Ftp_DELE:
	IF ARG(1)~="" THEN DO
		CALL WRITELN(1,"DELE "ARG(1))
		CALL Ftp_ReadLines
	END
	ELSE SAY "Argument needed!"
RETURN

Ftp_FEAT:
	CALL WRITELN(1,"FEAT")
	CALL Ftp_ReadLines
RETURN

Ftp_HELP:
	IF ARG(1)="" THEN DO
		CALL WRITELN(1,"HELP")
	END
	ELSE CALL WRITELN(1,"HELP "ARG(1))
	CALL Ftp_ReadLines
RETURN

Ftp_LCD:
	IF ARG(1)="" THEN SAY "Local dir: "download_to
	ELSE DO
		newdir=ARG(1)
		IF RIGHT(newdir,1)~=":"&RIGHT(newdir,1)~="/" THEN newdir=newdir"/"
		IF EXISTS(newdir) THEN download_to=newdir
	END
RETURN

Ftp_LIST:
	list_args=ARG(1)
	CALL Ftp_PASV
	IF result=0 THEN RETURN
	IF list_Args~="" THEN CALL WRITELN(1,"LIST "list_args)
	ELSE CALL WRITELN(1,"LIST")
	CALL Ftp_ReadLines

	IF LEFT(result,3)<400 THEN DO
		DO WHILE(~EOF(2))
			line=READLN(2)
			IF line~="" THEN SAY line
		END
		DROP line
	END
	CALL CLOSE(2)
	CALL Ftp_ReadLines
RETURN

Ftp_EPLF:
	list_args=ARG(1)
	CALL Ftp_PASV
	IF result=0 THEN RETURN
	IF list_Args~="" THEN CALL WRITELN(1,"EPLF "list_args)
	ELSE CALL WRITELN(1,"EPLF")
	CALL Ftp_ReadLines

	IF LEFT(result,3)<400 THEN DO
		DO WHILE(~EOF(2))
			line=READLN(2)
			IF line~="" THEN SAY line
		END
		DROP line
	END
	CALL CLOSE(2)
	CALL Ftp_ReadLines
RETURN

Ftp_MKD:
	IF ARG(1)~="" THEN DO
		CALL WRITELN(1,"MKD "ARG(1))
		CALL Ftp_ReadLines
	END
	ELSE SAY "Argument needed!"
RETURN

Ftp_NOOP:
	CALL WRITELN(1,"NOOP")
	CALL Ftp_ReadLines
RETURN

Ftp_PASV:
	success=0
	CALL WRITELN(1,"PASV")
	CALL Ftp_ReadLines
	res=result
	IF LEFT(res,3)<300 THEN DO
		success=1
		PARSE VAR res code . '(' n1 ',' n2 ',' n3 ',' n4 ',' n5 ',' n6 ')'
		n7 = (n5*256)+n6
		hostname_pasv = 'tcp:'n1'.'n2'.'n3'.'n4'/'n7
		IF ~OPEN(2,hostname_pasv,"RW") THEN DO
			SAY "PASV connection not opened!!!! Aborting..."
			success=0
		END
	END
RETURN(success)

Ftp_PORT:
	ip=GETHOSTID() /* rxsocket.library */
	ip=TRANSLATE(ip,',','.')
	CALL WRITELN(1,"PORT "||ip||",5,20")
	CALL Ftp_ReadLines
RETURN

Ftp_PWD:
	CALL WRITELN(1,"PWD")
	CALL Ftp_ReadLines
RETURN

Ftp_RAW:
	PARSE ARG raw_args
	CALL WRITELN(1,raw_args)
	CALL Ftp_ReadLines
RETURN

Ftp_RENA:
	rena_args=ARG(1)
	IF rena_args~="" THEN DO
		CALL WRITELN(1,"RNFR "WORD(rena_args,1))
		CALL Ftp_ReadLines
		/*
		350 1 exists, now send destination filename
		*/
		IF result>299&result<400 THEN DO
			CALL WRITELN(1,"RNTO "WORD(rena_args,2))
			CALL Ftp_ReadLines
			/*
			*/
		END
	END
	ELSE SAY "RENA <old_name> <new_name>"
RETURN

Ftp_REST:
	IF ARG(1)~="" THEN DO
		CALL WRITELN(1,"REST "ARG(1))
		CALL Ftp_ReadLines
	END
	ELSE SAY "Argument required!"
RETURN

Ftp_RETR:
	retr_file=ARG(1)
	IF retr_file~="" THEN DO
		IF EXISTS(download_to||retr_file) THEN DO
			SAY "File already exists! [O]verwrite, [R]esume or [A]bort: "
			PARSE PULL answer
		END
		ELSE answer="O"
		IF UPPER(answer)="R" THEN DO
			isitafile=WORD(STATEF(download_to||retr_file),1) /* rexxsupport.library */
			IF isitafile="FILE" THEN DO
				localfilesize=WORD(STATEF(download_to||retr_file),2) /* rexxsupport.library */
				CALL Ftp_REST(localfilesize)
				CALL Ftp_ReadLines
				resu=result
			END
		END
		IF UPPER(answer)="O" THEN DO
			CALL Ftp_PASV
			CALL WRITELN(1,"RETR "retr_file)
			CALL Ftp_ReadLines
			resu=result
		END
		IF LEFT(resu,3)<500 THEN DO
			IF UPPER(answer)="O" THEN DO
				dlsize=0
				CALL OPEN(3,download_to||retr_file,"W")
				DO WHILE(~EOF(2))
					tmpfile=READCH(2,1024)
					CALL WRITECH(3,tmpfile)
					dlsize=dlsize+LENGTH(tmpfile)
					ADDRESS COMMAND "echo noline "CR"("dlsize")"
				END
				SAY
				CALL CLOSE(3)
				CALL Ftp_ReadLines
			END
			IF UPPER(answer)="R" THEN DO
				IF isitafile="FILE" THEN DO
					CALL OPEN(3,download_to||retr_file,"A")
					DO WHILE(~EOF(2))
						CALL WRITECH(3,READCH(2,1024))
					END
					CALL CLOSE(3)
					CALL Ftp_ReadLines
				END
				ELSE SAY "The file doesn't exists on your HD, can't resume!"
			END
		END
		CALL CLOSE(2)
	END
	ELSE SAY "You must give the file to DL as an argument!"
RETURN

Ftp_RMD:
	IF ARG(1)~="" THEN DO
		CALL WRITELN(1,"RMD "ARG(1))
		CALL Ftp_ReadLines
	END
	ELSE SAY "Argument needed!"
RETURN

Ftp_STOR:
	stor_args=ARG(1)
	IF EXISTS(stor_args) THEN DO
		CALL Ftp_PASV
		stor_args2=stor_args
		IF LASTPOS(':',stor_args2)>0 THEN stor_args2=DELSTR(stor_args2,1,LASTPOS(':',stor_args2))
		IF LASTPOS('/',stor_args2)>0 THEN stor_args2=DELSTR(stor_args2,1,LASTPOS('/',stor_args2))
		IF stor_args2~="" THEN DO
			CALL WRITELN(1,"STOR "stor_args2)
			CALL Ftp_ReadLines
			IF LEFT(result,3)<500 THEN DO
				CALL OPEN(3,stor_args,"R")
				DO WHILE(~EOF(3))
					CALL WRITECH(2,READCH(3,1024))
				END
				CALL CLOSE(3)
				CALL Ftp_ReadLines
			END
			CALL CLOSE(2)
		END
		ELSE SAY "No file!"
	END
	ELSE SAY "File does not exist!"
RETURN

Ftp_SYST:
	CALL WRITELN(1,"SYST")
	CALL Ftp_ReadLines
RETURN

Ftp_TYPE:
	CALL WRITELN(1,"TYPE "ARG(1))
	CALL Ftp_ReadLines
RETURN

Ftp_QUIT:
	IF connected=1 THEN DO
		CALL WRITELN(1,"QUIT")
		CALL Ftp_ReadLines
		IF ~CLOSE(1) THEN SAY "No connection open..."
		connected=0
	END
	ELSE SAY "No Connection open..."
RETURN

Ftp_ReadLines:
	savednum=0
	DO FOREVER
		txt=READLN(1)
		IF savednum=0 THEN DO
			savednum=1
			linesnum=LEFT(txt,3)
		END
		SAY txt
		IF LEFT(txt,4)=linesnum||" " THEN LEAVE
	END
RETURN(txt)
