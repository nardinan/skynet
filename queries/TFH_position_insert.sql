INSERT INTO t_TFH_position(TFH_fk, location_fk, date) 
	SELECT * FROM (SELECT
		(SELECT t_TFH_device.TFH_fk FROM t_TFH_device LEFT JOIN t_device ON (t_TFH_device.device_fk = t_device.device_pk) WHERE
			(t_device.kind = #device_kind) AND (t_device.code = #device_code) AND (t_device.type = #device_type) AND (t_device.connector = #device_connector)) AS tmp_TFH_fk,
		(SELECT location_pk FROM t_location WHERE (code = #test_location_code)) AS tmp_location_fk,
		#test_date AS tmp_date) AS temporary WHERE NOT EXISTS
			(SELECT TFH_position_pk FROM t_TFH_position WHERE (TFH_fk = tmp_TFH_fk) AND (date = #test_date)) LIMIT 1;
