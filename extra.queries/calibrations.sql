SELECT
	t_device_measurement.channel,
	t_device_measurement.pedestal,
	t_device_measurement.sigma_raw,
	t_device_measurement.sigma,
	current_device.device_kind,
	current_device.location_code,
	current_device.device_type,
	current_device.device_code,
	current_device.connector,
	current_device.test_kind,
	current_device.test_date
FROM t_device_measurement INNER JOIN
	(SELECT
		t_device.kind AS device_kind,
		t_device.code AS device_code,
		t_device.type AS device_type,
		t_device.connector AS connector,
		t_location.code AS location_code,
		current_device_test.kind AS test_kind,
		current_device_test.date AS test_date,
		current_device_test.device_fk,
		current_device_test.device_test_pk AS device_test_pk
	FROM t_device LEFT JOIN
		(SELECT kind, date, device_fk FROM t_device_test AS device_test WHERE
			(date = (select MAX(date) FROM t_device_test WHERE 
					(kind = device_test.kind) AND
					(device_fk = device_test.device_fk)))) AS current_device_test
		ON (t_device.device_pk = current_device_test.device_fk) LEFT JOIN t_location 
		ON (t_device.location_fk = t_location.location_pk)) AS current_device
	ON (t_device_measurement.device_test_fk = current_device.device_test_pk);
