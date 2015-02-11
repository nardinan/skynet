INSERT INTO t_device_test(device_fk, location_fk, date, room, temperature_1, temperature_2, voltage, current, cal_file, pdf_file, kind) 
	SELECT * FROM (SELECT 
		(SELECT device_pk FROM t_device WHERE 
			(kind = #device_kind) AND 
			(code = #device_code) AND 
			(type = #device_type) AND 
			(connector = #device_connector)
			(location_fk = (SELECT location_pk FROM t_location WHERE code = #device_location_code))) AS tmp_device_fk, 
		(SELECT location_pk FROM t_location WHERE code = #test_location_code) AS tmp_location_fk, 
		#test_date AS tmp_date, 
		#test_room_code AS tmp_room,
		#test_temperature_1 AS tmp_temperature_1, 
		#test_temperature_2 AS tmp_temperature_2, 
		#test_voltage AS tmp_voltage, 
		#test_current AS tmp_current, 
		#test_cal_file AS tmp_cal_file, 
		#test_pdf_file AS tmp_pdf_file, 
		#test_kind AS tmp_kind) AS temporary WHERE NOT EXISTS 
			(SELECT device_test_pk FROM t_device_test WHERE cal_file = #test_cal_file) LIMIT 1;
