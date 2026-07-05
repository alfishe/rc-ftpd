#ifndef FTPD_CAT_H
#define FTPD_CAT_H


/****************************************************************************/


/* This file was created automatically by CatComp.
 * Do NOT edit by hand!
 */


#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif

#ifdef CATCOMP_ARRAY
#undef CATCOMP_NUMBERS
#undef CATCOMP_STRINGS
#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#endif

#ifdef CATCOMP_BLOCK
#undef CATCOMP_STRINGS
#define CATCOMP_STRINGS
#endif


/****************************************************************************/


#ifdef CATCOMP_NUMBERS

#define MSG_ERROR 1000
#define MSG_OK 1001
#define MSG_SAVE 1002
#define MSG_USE 1003
#define MSG_CANCEL 1004
#define MSG_KICK 1005
#define MSG_ABORT 1006
#define MSG_ADD 1007
#define MSG_REMOVE 1008
#define MSG_SORT 1009
#define MSG_BAD_CONFIG 1010
#define MSG_STARTUP_MUI 1011
#define MSG_STARTUP_UTILITY 1012
#define MSG_STARTUP_SOCKET 1013
#define MSG_STARTUP_MEMORY 1014
#define MSG_STARTUP_APP 1015
#define MSG_SOCKET_OPEN 1016
#define MSG_SOCKET_OPTIONS 1017
#define MSG_SOCKET_BIND 1018
#define MSG_SOCKET_LISTEN 1019
#define MSG_SAVE_CONFIG 1020
#define MSG_LIST_STATUS 1021
#define MSG_LIST_IP 1022
#define MSG_LIST_USER 1023
#define MSG_LIST_CMD 1024
#define MSG_LIST_ARGS 1025
#define MSG_LIST_ONLINE 1026
#define MSG_MUI_ABOUT 1200
#define MSG_MUI_MUI 1201
#define MSG_MUI_PREFS 1202
#define MSG_SHORT_KICKABORT 1203
#define MSG_SHORT_MESSAGE 1204
#define MSG_SHORT_OPEN 1205
#define MSG_UNLIMITED 1206
#define MSG_CONTROL_CODES 1207
#define MSG_UPLOAD 1208
#define MSG_DOWNLOAD 1209
#define MSG_NOLOAD 1210
#define MSG_LOGIN 1211
#define MSG_LOG_NONE 1212
#define MSG_LOG_KNOWN 1213
#define MSG_LOG_COMMANDS 1214
#define MSG_LOG_ALL 1215
#define MSG_ONLINE_LIMIT 1216
#define MSG_ONLINE_NOLIMIT 1217
#define MSG_FTP_CLOSED 1218
#define MSG_FTP_OPEN 1219
#define MSG_FTP_GUIDE 1220
#define MSG_PREF_WINDOW 1221
#define MSG_PREF_PORT 1222
#define MSG_PREF_PORT_HELP 1223
#define MSG_PREF_LIST 1224
#define MSG_PREF_LIST_HELP 1225
#define MSG_PREF_ANON 1226
#define MSG_PREF_ANON_HELP 1227
#define MSG_PREF_MAXUSERS 1228
#define MSG_PREF_MAXUSERS_HELP 1229
#define MSG_PREF_MAXCPS 1230
#define MSG_PREF_MAXCPS_HELP 1231
#define MSG_PREF_TIMEOUT 1232
#define MSG_PREF_TIMEOUT_HELP 1233
#define MSG_PREF_NOOP 1234
#define MSG_PREF_NOOP_HELP 1235
#define MSG_PREF_NOPASV 1236
#define MSG_PREF_NOPASV_HELP 1237
#define MSG_PREF_AUTOMESSAGE 1238
#define MSG_PREF_AUTOMESSAGE_HELP 1239
#define MSG_PREF_AUTOCOMMENT 1240
#define MSG_PREF_AUTOCOMMENT_HELP 1241
#define MSG_PREF_LOGFILE 1242
#define MSG_PREF_LOGFILE_SEPERATE 1243
#define MSG_PREF_LOGFILE_HELP 1244
#define MSG_PAGE_INFO 1245
#define MSG_PAGE_USERS 1246
#define MSG_PAGE_GROUPS 1247
#define MSG_PAGE_GENERAL 1248
#define MSG_PAGE_CLOSED 1249
#define MSG_PAGE_GREETING 1250
#define MSG_PAGE_WELCOME 1251
#define MSG_PAGE_GOODBYE 1252
#define MSG_SNOOP_TITLE 1253
#define MSG_SNOOP_CLEAR 1254
#define MSG_USER_LIST_USERNAME 1255
#define MSG_USER_LIST_PASSWORD 1256
#define MSG_USER_LIST_MAX 1257
#define MSG_USER_LIST_LOG 1258
#define MSG_USER_LIST_NONAME 1259
#define MSG_USER_LIST_PASS 1260
#define MSG_USER_LIST_NOPASS 1261
#define MSG_USER_USER 1262
#define MSG_USER_PASS 1263
#define MSG_USER_MAX 1264
#define MSG_PATH_LIST_ALIAS 1265
#define MSG_PATH_LIST_PATH 1266
#define MSG_PATH_NOALIAS 1267
#define MSG_PATH_NOPATH 1268
#define MSG_PATH_READ 1269
#define MSG_PATH_WRITE 1270
#define MSG_PATH_DELETE 1271
#define MSG_PATH_SUBDIRS 1272
#define MSG_PATH_ALIAS 1273
#define MSG_PATH_PATH 1274
#define MSG_MENU_PROJECT 1275
#define MSG_MENU_ENABLED 1276
#define MSG_MENU_SETTINGS 1277
#define MSG_MENU_MUISETTINGS 1278
#define MSG_MENU_QUIT 1279
#define MSG_MUI_STATS 1280
#define MSG_STATS_TITLE 1281
#define MSG_STATS_DATA 1282
#define MSG_STATS_DATA_BYTES 1283
#define MSG_STATS_FILES 1284
#define MSG_STATS_FILES_FILES 1285
#define MSG_STATS_CPS 1286
#define MSG_STATS_CPS_CPS 1287
#define MSG_STATS_SENT 1288
#define MSG_STATS_RECIEVED 1289
#define MSG_STATS_TOTAL 1290
#define MSG_STATS_USERS 1291
#define MSG_STATS_USERS_ACCOUNT 1292
#define MSG_STATS_USERS_ANON 1293
#define MSG_STATS_RESET 1294
#define MSG_PREF_ASYNC 1295
#define MSG_PREF_ASYNC_HELP 1296

#endif /* CATCOMP_NUMBERS */


/****************************************************************************/


#ifdef CATCOMP_STRINGS

#define MSG_ERROR_STR "RC-FTPD Error"
#define MSG_OK_STR "Ok"
#define MSG_SAVE_STR "Save"
#define MSG_USE_STR "Use"
#define MSG_CANCEL_STR "Cancel"
#define MSG_KICK_STR "Kick"
#define MSG_ABORT_STR "Abort"
#define MSG_ADD_STR "Add"
#define MSG_REMOVE_STR "Remove"
#define MSG_SORT_STR "Sort"
#define MSG_BAD_CONFIG_STR "Unrecognised config file format"
#define MSG_STARTUP_MUI_STR "Unable to open muimaster.library 18+"
#define MSG_STARTUP_UTILITY_STR "Unable to open utility.library 36+"
#define MSG_STARTUP_SOCKET_STR "Unable to open bsdsocket.library 4+\n\nPlease start your TCP/IP stack first..."
#define MSG_STARTUP_MEMORY_STR "Unable to create memory pool\n\nQuit one or more applications or reboot"
#define MSG_STARTUP_APP_STR "Failed to create application\n\nPossibly a lack of memory or missing classes\n(more detail in later version)"
#define MSG_SOCKET_OPEN_STR "Unable to open socket\nUnknown cause - possibly unsuported socket\n\n(email me so I can trace it)"
#define MSG_SOCKET_OPTIONS_STR "Unable to set socket options\nUnknown cause - possibly unsuported socket"
#define MSG_SOCKET_BIND_STR "Unable to bind port\nProbably trying to start twice"
#define MSG_SOCKET_LISTEN_STR "Unable to listen on port\nUnknown cause - possibly unsuported socket"
#define MSG_SAVE_CONFIG_STR "Unable to save config"
#define MSG_LIST_STATUS_STR "Status"
#define MSG_LIST_IP_STR "IP"
#define MSG_LIST_USER_STR "User"
#define MSG_LIST_CMD_STR "CMD"
#define MSG_LIST_ARGS_STR "Args"
#define MSG_LIST_ONLINE_STR "\ec%ld/%ld Users online"
#define MSG_MUI_ABOUT_STR "The best ftp daemon for Amiga"
#define MSG_MUI_MUI_STR "MUI..."
#define MSG_MUI_PREFS_STR "Prefs..."
#define MSG_SHORT_KICKABORT_STR "\ebKick\en will disconnect the selected\nuser and any transfers they are doing\n\ebAbort\en will just cancel their transfer"
#define MSG_SHORT_MESSAGE_STR "Type a short message in here and it\nwill be sent to the selected user"
#define MSG_SHORT_OPEN_STR "This is the general status of your server\nCan people connect (\ebOpen\en) or not (\ebClosed\en)"
#define MSG_UNLIMITED_STR "unlimited"
#define MSG_CONTROL_CODES_STR "You can use the following control codes:\n$u = Current users\n$m = Max users\n$c = Max CPS\n$s = MaxCPS/Current users"
#define MSG_UPLOAD_STR "%s [%s]\n%s\n%s\n%s (%ld/%ld bytes, %lu cps)"
#define MSG_DOWNLOAD_STR "%s [%s]\n%s\n%s\n%s (%ld bytes, %lu cps)"
#define MSG_NOLOAD_STR "%s [%s]\n%s\n%s\n"
#define MSG_LOGIN_STR "\eblogin"
#define MSG_LOG_NONE_STR "No ftpd_log"
#define MSG_LOG_KNOWN_STR "Log known"
#define MSG_LOG_COMMANDS_STR "Log commands"
#define MSG_LOG_ALL_STR "Log all"
#define MSG_ONLINE_LIMIT_STR "\ec%ld/%ld Users online"
#define MSG_ONLINE_NOLIMIT_STR "\ec%ld Users online"
#define MSG_FTP_CLOSED_STR "FTP Closed"
#define MSG_FTP_OPEN_STR "FTP Open"
#define MSG_FTP_GUIDE_STR "ftpd.Guide"
#define MSG_PREF_WINDOW_STR "RC-FTPd Preferences"
#define MSG_PREF_PORT_STR "Port:"
#define MSG_PREF_PORT_HELP_STR "This is the port people must connect to,\ntakes effect on restart.\n\nDefault: 21"
#define MSG_PREF_LIST_STR "List pattern:"
#define MSG_PREF_LIST_HELP_STR "This is the pattern used to list files/dirs,\nit does not stop download, just visibility.\n\nDefault: \"~(.#?|#?.info)\""
#define MSG_PREF_ANON_STR "Anon:"
#define MSG_PREF_ANON_HELP_STR "Do you allow people to ftpd_log on \ebwithout\en passwords?\n\nDefault: NO\nNote: they must have a blank password on the Users page."
#define MSG_PREF_MAXUSERS_STR "Max Users:"
#define MSG_PREF_MAXUSERS_HELP_STR "The maximum number of control connections,\nsome clients make more than one\nconnection, so you should also set a Timeout.\n\nDefault: 0 (no limit)"
#define MSG_PREF_MAXCPS_STR "Max CPS:"
#define MSG_PREF_MAXCPS_HELP_STR "The maximum transfer speed for files,\nuseful if you are on a modem and\ndon't want the server stealing\nall your bandwidth.\n\nDefault: 0 (no limit)"
#define MSG_PREF_TIMEOUT_STR "Timeout:"
#define MSG_PREF_TIMEOUT_HELP_STR "After this many minutes of doing nothing\npeople are kicked off.\n\nDefault: 0 (no timeout)"
#define MSG_PREF_NOOP_STR "Noop:"
#define MSG_PREF_NOOP_HELP_STR "If this is allowed then people can\nuse the NOOP command (NO OPeration) to\nprevent themselves being kicked off.\n\nDefault: OFF"
#define MSG_PREF_NOPASV_STR "NoPASV:"
#define MSG_PREF_NOPASV_HELP_STR "If this is checked then people can\nonly connect via the PORT command.\nThis is more secure,\nand recommended for people behind firewalls."
#define MSG_PREF_AUTOMESSAGE_STR "Auto-Message:"
#define MSG_PREF_AUTOMESSAGE_HELP_STR "Whenever someone changes their directory\nthis will type a file called \".message\"\nas part of the reply they get.\n\nDefault: ON"
#define MSG_PREF_AUTOCOMMENT_STR "Auto-Comment:"
#define MSG_PREF_AUTOCOMMENT_HELP_STR "Whenever someone sends you a file this\nwill set the file comment to \"user@ip\".\n\nDefault: ON (\ebRegistered only\en)"
#define MSG_PREF_LOGFILE_STR "Use-Logfile:"
#define MSG_PREF_LOGFILE_SEPERATE_STR "Seperate:"
#define MSG_PREF_LOGFILE_HELP_STR "Logs everything you chose (as in Snoop)\nto either a single logfile, or one\nlogfile per user."
#define MSG_PAGE_INFO_STR "Info"
#define MSG_PAGE_USERS_STR "Users"
#define MSG_PAGE_GROUPS_STR "Groups"
#define MSG_PAGE_GENERAL_STR "General"
#define MSG_PAGE_CLOSED_STR "\e3Closed"
#define MSG_PAGE_GREETING_STR "\e3Greeting"
#define MSG_PAGE_WELCOME_STR "\e3Welcome"
#define MSG_PAGE_GOODBYE_STR "\e3Goodbye"
#define MSG_SNOOP_TITLE_STR "RC-FTPd Snoop"
#define MSG_SNOOP_CLEAR_STR "Clear"
#define MSG_USER_LIST_USERNAME_STR "Username"
#define MSG_USER_LIST_PASSWORD_STR "Password"
#define MSG_USER_LIST_MAX_STR "Max"
#define MSG_USER_LIST_LOG_STR "Log"
#define MSG_USER_LIST_NONAME_STR "\ebunset"
#define MSG_USER_LIST_PASS_STR "(hidden)"
#define MSG_USER_LIST_NOPASS_STR "\ebanon"
#define MSG_USER_USER_STR "User:"
#define MSG_USER_PASS_STR "Pass:"
#define MSG_USER_MAX_STR "Max:"
#define MSG_PATH_LIST_ALIAS_STR "Alias"
#define MSG_PATH_LIST_PATH_STR "Path"
#define MSG_PATH_NOALIAS_STR "\ebnone"
#define MSG_PATH_NOPATH_STR "\ebfake"
#define MSG_PATH_READ_STR "Read:"
#define MSG_PATH_WRITE_STR "Write:"
#define MSG_PATH_DELETE_STR "Delete:"
#define MSG_PATH_SUBDIRS_STR "SubDirs:"
#define MSG_PATH_ALIAS_STR "Alias:"
#define MSG_PATH_PATH_STR "Path:"
#define MSG_MENU_PROJECT_STR "Project"
#define MSG_MENU_ENABLED_STR "Enabled"
#define MSG_MENU_SETTINGS_STR "Settings..."
#define MSG_MENU_MUISETTINGS_STR "MUI Settings..."
#define MSG_MENU_QUIT_STR "Quit"
#define MSG_MUI_STATS_STR "Stats"
#define MSG_STATS_TITLE_STR "RC-FTPd Stats"
#define MSG_STATS_DATA_STR "Data Transferred"
#define MSG_STATS_DATA_BYTES_STR "%s bytes"
#define MSG_STATS_FILES_STR "Files Transferred"
#define MSG_STATS_FILES_FILES_STR "%s file(s)"
#define MSG_STATS_CPS_STR "Average CPS"
#define MSG_STATS_CPS_CPS_STR "%s cps"
#define MSG_STATS_SENT_STR "Sent:"
#define MSG_STATS_RECIEVED_STR "Recieved:"
#define MSG_STATS_TOTAL_STR "Total:"
#define MSG_STATS_USERS_STR "Users"
#define MSG_STATS_USERS_ACCOUNT_STR "Account:"
#define MSG_STATS_USERS_ANON_STR "Anon:"
#define MSG_STATS_RESET_STR "Reset Stats"
#define MSG_PREF_ASYNC_STR "Async"
#define MSG_PREF_ASYNC_HELP_STR "With this enabled file transfer is faster and uses less CPU...\nHowever it has been reported to crash some people's\nmachines - you have been warned\n\nDefault: OFF"

#endif /* CATCOMP_STRINGS */


/****************************************************************************/


#ifdef CATCOMP_ARRAY

struct CatCompArrayType
{
    LONG   cca_ID;
    STRPTR cca_Str;
};

static const struct CatCompArrayType CatCompArray[] =
{
    {MSG_ERROR,(STRPTR)MSG_ERROR_STR},
    {MSG_OK,(STRPTR)MSG_OK_STR},
    {MSG_SAVE,(STRPTR)MSG_SAVE_STR},
    {MSG_USE,(STRPTR)MSG_USE_STR},
    {MSG_CANCEL,(STRPTR)MSG_CANCEL_STR},
    {MSG_KICK,(STRPTR)MSG_KICK_STR},
    {MSG_ABORT,(STRPTR)MSG_ABORT_STR},
    {MSG_ADD,(STRPTR)MSG_ADD_STR},
    {MSG_REMOVE,(STRPTR)MSG_REMOVE_STR},
    {MSG_SORT,(STRPTR)MSG_SORT_STR},
    {MSG_BAD_CONFIG,(STRPTR)MSG_BAD_CONFIG_STR},
    {MSG_STARTUP_MUI,(STRPTR)MSG_STARTUP_MUI_STR},
    {MSG_STARTUP_UTILITY,(STRPTR)MSG_STARTUP_UTILITY_STR},
    {MSG_STARTUP_SOCKET,(STRPTR)MSG_STARTUP_SOCKET_STR},
    {MSG_STARTUP_MEMORY,(STRPTR)MSG_STARTUP_MEMORY_STR},
    {MSG_STARTUP_APP,(STRPTR)MSG_STARTUP_APP_STR},
    {MSG_SOCKET_OPEN,(STRPTR)MSG_SOCKET_OPEN_STR},
    {MSG_SOCKET_OPTIONS,(STRPTR)MSG_SOCKET_OPTIONS_STR},
    {MSG_SOCKET_BIND,(STRPTR)MSG_SOCKET_BIND_STR},
    {MSG_SOCKET_LISTEN,(STRPTR)MSG_SOCKET_LISTEN_STR},
    {MSG_SAVE_CONFIG,(STRPTR)MSG_SAVE_CONFIG_STR},
    {MSG_LIST_STATUS,(STRPTR)MSG_LIST_STATUS_STR},
    {MSG_LIST_IP,(STRPTR)MSG_LIST_IP_STR},
    {MSG_LIST_USER,(STRPTR)MSG_LIST_USER_STR},
    {MSG_LIST_CMD,(STRPTR)MSG_LIST_CMD_STR},
    {MSG_LIST_ARGS,(STRPTR)MSG_LIST_ARGS_STR},
    {MSG_LIST_ONLINE,(STRPTR)MSG_LIST_ONLINE_STR},
    {MSG_MUI_ABOUT,(STRPTR)MSG_MUI_ABOUT_STR},
    {MSG_MUI_MUI,(STRPTR)MSG_MUI_MUI_STR},
    {MSG_MUI_PREFS,(STRPTR)MSG_MUI_PREFS_STR},
    {MSG_SHORT_KICKABORT,(STRPTR)MSG_SHORT_KICKABORT_STR},
    {MSG_SHORT_MESSAGE,(STRPTR)MSG_SHORT_MESSAGE_STR},
    {MSG_SHORT_OPEN,(STRPTR)MSG_SHORT_OPEN_STR},
    {MSG_UNLIMITED,(STRPTR)MSG_UNLIMITED_STR},
    {MSG_CONTROL_CODES,(STRPTR)MSG_CONTROL_CODES_STR},
    {MSG_UPLOAD,(STRPTR)MSG_UPLOAD_STR},
    {MSG_DOWNLOAD,(STRPTR)MSG_DOWNLOAD_STR},
    {MSG_NOLOAD,(STRPTR)MSG_NOLOAD_STR},
    {MSG_LOGIN,(STRPTR)MSG_LOGIN_STR},
    {MSG_LOG_NONE,(STRPTR)MSG_LOG_NONE_STR},
    {MSG_LOG_KNOWN,(STRPTR)MSG_LOG_KNOWN_STR},
    {MSG_LOG_COMMANDS,(STRPTR)MSG_LOG_COMMANDS_STR},
    {MSG_LOG_ALL,(STRPTR)MSG_LOG_ALL_STR},
    {MSG_ONLINE_LIMIT,(STRPTR)MSG_ONLINE_LIMIT_STR},
    {MSG_ONLINE_NOLIMIT,(STRPTR)MSG_ONLINE_NOLIMIT_STR},
    {MSG_FTP_CLOSED,(STRPTR)MSG_FTP_CLOSED_STR},
    {MSG_FTP_OPEN,(STRPTR)MSG_FTP_OPEN_STR},
    {MSG_FTP_GUIDE,(STRPTR)MSG_FTP_GUIDE_STR},
    {MSG_PREF_WINDOW,(STRPTR)MSG_PREF_WINDOW_STR},
    {MSG_PREF_PORT,(STRPTR)MSG_PREF_PORT_STR},
    {MSG_PREF_PORT_HELP,(STRPTR)MSG_PREF_PORT_HELP_STR},
    {MSG_PREF_LIST,(STRPTR)MSG_PREF_LIST_STR},
    {MSG_PREF_LIST_HELP,(STRPTR)MSG_PREF_LIST_HELP_STR},
    {MSG_PREF_ANON,(STRPTR)MSG_PREF_ANON_STR},
    {MSG_PREF_ANON_HELP,(STRPTR)MSG_PREF_ANON_HELP_STR},
    {MSG_PREF_MAXUSERS,(STRPTR)MSG_PREF_MAXUSERS_STR},
    {MSG_PREF_MAXUSERS_HELP,(STRPTR)MSG_PREF_MAXUSERS_HELP_STR},
    {MSG_PREF_MAXCPS,(STRPTR)MSG_PREF_MAXCPS_STR},
    {MSG_PREF_MAXCPS_HELP,(STRPTR)MSG_PREF_MAXCPS_HELP_STR},
    {MSG_PREF_TIMEOUT,(STRPTR)MSG_PREF_TIMEOUT_STR},
    {MSG_PREF_TIMEOUT_HELP,(STRPTR)MSG_PREF_TIMEOUT_HELP_STR},
    {MSG_PREF_NOOP,(STRPTR)MSG_PREF_NOOP_STR},
    {MSG_PREF_NOOP_HELP,(STRPTR)MSG_PREF_NOOP_HELP_STR},
    {MSG_PREF_NOPASV,(STRPTR)MSG_PREF_NOPASV_STR},
    {MSG_PREF_NOPASV_HELP,(STRPTR)MSG_PREF_NOPASV_HELP_STR},
    {MSG_PREF_AUTOMESSAGE,(STRPTR)MSG_PREF_AUTOMESSAGE_STR},
    {MSG_PREF_AUTOMESSAGE_HELP,(STRPTR)MSG_PREF_AUTOMESSAGE_HELP_STR},
    {MSG_PREF_AUTOCOMMENT,(STRPTR)MSG_PREF_AUTOCOMMENT_STR},
    {MSG_PREF_AUTOCOMMENT_HELP,(STRPTR)MSG_PREF_AUTOCOMMENT_HELP_STR},
    {MSG_PREF_LOGFILE,(STRPTR)MSG_PREF_LOGFILE_STR},
    {MSG_PREF_LOGFILE_SEPERATE,(STRPTR)MSG_PREF_LOGFILE_SEPERATE_STR},
    {MSG_PREF_LOGFILE_HELP,(STRPTR)MSG_PREF_LOGFILE_HELP_STR},
    {MSG_PAGE_INFO,(STRPTR)MSG_PAGE_INFO_STR},
    {MSG_PAGE_USERS,(STRPTR)MSG_PAGE_USERS_STR},
    {MSG_PAGE_GROUPS,(STRPTR)MSG_PAGE_GROUPS_STR},
    {MSG_PAGE_GENERAL,(STRPTR)MSG_PAGE_GENERAL_STR},
    {MSG_PAGE_CLOSED,(STRPTR)MSG_PAGE_CLOSED_STR},
    {MSG_PAGE_GREETING,(STRPTR)MSG_PAGE_GREETING_STR},
    {MSG_PAGE_WELCOME,(STRPTR)MSG_PAGE_WELCOME_STR},
    {MSG_PAGE_GOODBYE,(STRPTR)MSG_PAGE_GOODBYE_STR},
    {MSG_SNOOP_TITLE,(STRPTR)MSG_SNOOP_TITLE_STR},
    {MSG_SNOOP_CLEAR,(STRPTR)MSG_SNOOP_CLEAR_STR},
    {MSG_USER_LIST_USERNAME,(STRPTR)MSG_USER_LIST_USERNAME_STR},
    {MSG_USER_LIST_PASSWORD,(STRPTR)MSG_USER_LIST_PASSWORD_STR},
    {MSG_USER_LIST_MAX,(STRPTR)MSG_USER_LIST_MAX_STR},
    {MSG_USER_LIST_LOG,(STRPTR)MSG_USER_LIST_LOG_STR},
    {MSG_USER_LIST_NONAME,(STRPTR)MSG_USER_LIST_NONAME_STR},
    {MSG_USER_LIST_PASS,(STRPTR)MSG_USER_LIST_PASS_STR},
    {MSG_USER_LIST_NOPASS,(STRPTR)MSG_USER_LIST_NOPASS_STR},
    {MSG_USER_USER,(STRPTR)MSG_USER_USER_STR},
    {MSG_USER_PASS,(STRPTR)MSG_USER_PASS_STR},
    {MSG_USER_MAX,(STRPTR)MSG_USER_MAX_STR},
    {MSG_PATH_LIST_ALIAS,(STRPTR)MSG_PATH_LIST_ALIAS_STR},
    {MSG_PATH_LIST_PATH,(STRPTR)MSG_PATH_LIST_PATH_STR},
    {MSG_PATH_NOALIAS,(STRPTR)MSG_PATH_NOALIAS_STR},
    {MSG_PATH_NOPATH,(STRPTR)MSG_PATH_NOPATH_STR},
    {MSG_PATH_READ,(STRPTR)MSG_PATH_READ_STR},
    {MSG_PATH_WRITE,(STRPTR)MSG_PATH_WRITE_STR},
    {MSG_PATH_DELETE,(STRPTR)MSG_PATH_DELETE_STR},
    {MSG_PATH_SUBDIRS,(STRPTR)MSG_PATH_SUBDIRS_STR},
    {MSG_PATH_ALIAS,(STRPTR)MSG_PATH_ALIAS_STR},
    {MSG_PATH_PATH,(STRPTR)MSG_PATH_PATH_STR},
    {MSG_MENU_PROJECT,(STRPTR)MSG_MENU_PROJECT_STR},
    {MSG_MENU_ENABLED,(STRPTR)MSG_MENU_ENABLED_STR},
    {MSG_MENU_SETTINGS,(STRPTR)MSG_MENU_SETTINGS_STR},
    {MSG_MENU_MUISETTINGS,(STRPTR)MSG_MENU_MUISETTINGS_STR},
    {MSG_MENU_QUIT,(STRPTR)MSG_MENU_QUIT_STR},
    {MSG_MUI_STATS,(STRPTR)MSG_MUI_STATS_STR},
    {MSG_STATS_TITLE,(STRPTR)MSG_STATS_TITLE_STR},
    {MSG_STATS_DATA,(STRPTR)MSG_STATS_DATA_STR},
    {MSG_STATS_DATA_BYTES,(STRPTR)MSG_STATS_DATA_BYTES_STR},
    {MSG_STATS_FILES,(STRPTR)MSG_STATS_FILES_STR},
    {MSG_STATS_FILES_FILES,(STRPTR)MSG_STATS_FILES_FILES_STR},
    {MSG_STATS_CPS,(STRPTR)MSG_STATS_CPS_STR},
    {MSG_STATS_CPS_CPS,(STRPTR)MSG_STATS_CPS_CPS_STR},
    {MSG_STATS_SENT,(STRPTR)MSG_STATS_SENT_STR},
    {MSG_STATS_RECIEVED,(STRPTR)MSG_STATS_RECIEVED_STR},
    {MSG_STATS_TOTAL,(STRPTR)MSG_STATS_TOTAL_STR},
    {MSG_STATS_USERS,(STRPTR)MSG_STATS_USERS_STR},
    {MSG_STATS_USERS_ACCOUNT,(STRPTR)MSG_STATS_USERS_ACCOUNT_STR},
    {MSG_STATS_USERS_ANON,(STRPTR)MSG_STATS_USERS_ANON_STR},
    {MSG_STATS_RESET,(STRPTR)MSG_STATS_RESET_STR},
    {MSG_PREF_ASYNC,(STRPTR)MSG_PREF_ASYNC_STR},
    {MSG_PREF_ASYNC_HELP,(STRPTR)MSG_PREF_ASYNC_HELP_STR},
};

#endif /* CATCOMP_ARRAY */


/****************************************************************************/


struct LocaleInfo
{
    APTR li_LocaleBase;
    APTR li_Catalog;
};



#endif /* FTPD_CAT_H */
