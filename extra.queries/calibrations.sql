SELECT 
	device.kind 			AS device_kind,
	location.code 			AS device_location, 
	device.type 			AS device_type, 
	device.code 			AS device_code, 
	device_test.date 		AS test_date, 
	device_test.kind 		AS test_kind,
FROM t_device_test			AS device_test
	LEFT JOIN t_device 		AS device 	ON (device.device_pk = device_test.device_fk)
	LEFT JOIN t_location 		AS location 	ON (location.location_pk = device.location_fk);
