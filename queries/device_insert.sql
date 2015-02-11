INSERT INTO t_device(location_fk, kind, code, type, connector) 
	SELECT * FROM (SELECT
		(SELECT location_pk FROM t_location WHERE code = #device_location_code) AS tmp_location_fk, 
		#device_kind AS tmp_kind, 
		#device_code AS tmp_code, 
		#device_type AS tmp_type, 
		#device_connector AS tmp_connector) AS temporary WHERE NOT EXISTS 
			(SELECT device_pk FROM t_device WHERE 
				(kind = #device_kind) AND 
				(code = #device_code) AND 
				(type = #device_type) AND 
				(connector = #device_connector) AND 
				(location_fk = tmp_location_fk)) LIMIT 1;
