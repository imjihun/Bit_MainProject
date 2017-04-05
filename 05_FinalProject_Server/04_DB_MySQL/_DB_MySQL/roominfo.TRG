TYPE=TRIGGERS
triggers='CREATE DEFINER=`root`@`localhost` TRIGGER `mysql_test`.`roominfo_AFTER_UPDATE` AFTER UPDATE ON `roominfo` FOR EACH ROW\nBEGIN\n	INSERT INTO roomresultinfo (Room_Number, RoomResult_NullityCnt) SELECT NEW.Room_Number, NEW.Room_CurMemberCount FROM DUAL WHERE EXISTS (SELECT * FROM roominfo WHERE Room_Number=NEW.Room_Number AND OLD.Room_State=0 AND NEW.Room_State=1);\n    \n	DELETE FROM roomresultinfo WHERE Room_Number=NEW.Room_Number AND OLD.Room_State=1 AND NEW.Room_State=0;\nEND'
sql_modes=1344274432
definers='root@localhost'
client_cs_names='utf8'
connection_cl_names='utf8_general_ci'
db_cl_names='dec8_swedish_ci'
created=148240450971
