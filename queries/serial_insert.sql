INSERT INTO t_sensor(TFH_fk, serial)
	SELECT * FROM (SELECT
		#device_code AS tmp_TFH_fk,
		#test_serial AS tmp_serial) AS temporary WHERE NOT EXISTS
			(SELECT serial FROM t_sensor WHERE (serial = #test_serial)) AND ("H" = #device_kind) LIMIT 1
