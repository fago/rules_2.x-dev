<?php
// $Id$

/**
 * @file Rules Admin UI - Utility functions
 */

/**
 * Returns data from rules_fetch_data sorted by groups
 */
function rules_admin_get_grouped_data($hook) {
	$sorted_data = array();
	$data = rules_fetch_data($hook);
	foreach ($data as $name => $set) {
			$sorted_data[ucfirst($set['group'])][$name] = $set['label'];
	}
	foreach ($sorted_data as $key => $set) {
		asort($set);
		$sorted_data[$key] = $set;
	}
	return $sorted_data;	
}