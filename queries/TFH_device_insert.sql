INSERT INTO t_TFH_device(TFH_fk, device_fk)
	SELECT * FROM (SELECT
		(SELECT TFH_fk FROM t_sensor WHERE (serial = #test_serial)) as tmp_TFH_fk,
		(SELECT device_pk FROM t_device WHERE (kind = #device_kind) AND (code = #device_code) AND (type = #device_type) AND (connector = #device_connector)) as tmp_device_fk) AS temporary WHERE
			EXISTS (SELECT serial FROM t_sensor WHERE (serial = #test_serial)) AND
			NOT EXISTS (SELECT TFH_fk, device_fk FROM t_TFH_device WHERE (TFH_fk = tmp_TFH_fk) AND (device_fk = tmp_device_fk)) LIMIT 1;
