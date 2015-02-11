INSERT INTO t_device_measurement(device_test_fk, channel, pedestal, sigma_raw, sigma, bad_channel) 
	SELECT * FROM (SELECT 
		(SELECT device_test_pk FROM t_device_test WHERE cal_file = #test_cal_file) AS tmp_device_test_fk, 
		#test_channel AS tmp_channel, 
		#test_pedestal AS tmp_pedestal, 
		#test_sigma_raw AS tmp_sigma_raw,
		#test_sigma AS tmp_sigma, 
		#test_bad_channel AS tmp_bad_channel) AS temporary WHERE NOT EXISTS 
			(SELECT t_device_measurement.device_measurement_pk FROM t_device_measurement LEFT JOIN t_device_test ON 
				(t_device_measurement.device_test_fk = t_device_test.device_test_pk) WHERE
				(t_device_test.cal_file = #test_cal_file) AND
				(t_device_measurement.channel = #test_channel)) LIMIT 1;
