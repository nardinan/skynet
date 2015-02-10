SELECT t_device.kind, t_location.code, t_device.type, t_device.code, t_device_measurement.channel, t_device_measurement.pedestal, 
	t_device_measurement.sigma_raw, t_device_measurement.sigma 
FROM t_device_measurement
	LEFT JOIN t_device_test ON (t_device_test.device_test_pk = t_device_measurement.device_test_fk)
	LEFT JOIN t_device ON (t_device.device_pk = t_device_test.device_fk)
	LEFT JOIN t_location ON (t_location.location_pk = t_device.location_fk);
