INSERT INTO t_TFH(TFH_pk)
	SELECT * FROM (SELECT
		#device_code AS tmp_TFH_pk) AS temporary WHERE NOT EXISTS
			(SELECT TFH_pk FROM t_TFH WHERE TFH_pk = #device_code) AND ("H" = #device_kind) LIMIT 1;
