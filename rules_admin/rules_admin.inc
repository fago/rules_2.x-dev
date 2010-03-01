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


function rules_admin_overview_table($conditions = array()) {
  $rules = rules_config_load_multiple(FALSE, $conditions);
  $rows = array();
  $events = rules_fetch_data('event_info');

  foreach ($rules as $name => $rule) {
    $path = RULES_ADMIN_RULE_PATH . '/' . $name;
    $row = array();
    $row[] = l($rule->label, $path);

    $rule_events = array();

    foreach ($rule->events() as $ename)
    {
      $rule_events[] = $events[$ename]['label'];
    }
      $row[] = implode(", ", $rule_events);
      $row[] = l(t('clone'), $path . '/clone', array(), drupal_get_destination());
      $row[] = l(t('delete'), $path . '/delete', array(), drupal_get_destination());
      $rows[] = $row;
  }
  $header = array(t('Label'), array('data' => t('Event'), 'colspan' => 1), array('data' => t('Operations'), 'colspan' => '2'));

  $table = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#empty' => t('No reaction rules defined yet.'),
  );

  return $table;
}



