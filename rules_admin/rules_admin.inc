<?php
// $Id$

/**
 * @file Rules Admin UI - Utility functions
 */

/**
 * Extracts the property with the given name while keeping the keys
 *
 * @param $key The name of the property to extract
 * @param $elements An array of elements
 *
 * @return An array of extracted properties.
 */
function rules_extract_property($elements, $key) {
  $data = array();
  foreach ($elements as $name => $info) {
    $data[$name] = $info[$key];
  }
  return $data;
}


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

function theme_rules_elements($variables) {
  $form = $variables['form'];

  drupal_add_css(drupal_get_path('module', 'rules_admin') . '/rules.css');

  $output = '';
  $output .= '<div class="rules_element ui-draggable">';
  $output .= '<div class="rules_element_head">';
  $output .= drupal_render($form['head']);
  $output .= '</div>';
  $output .= '<div class="rules_element_operator">';
  $output .= '<div class="rules_element_edit">';
  $output .= drupal_render($form['operator']['edit']);
  $output .= '</div>';
  $output .= '<div class="rules_element_delete">';
  $output .= drupal_render($form['operator']['delete']);
  $output .= '</div>';
  $output .= '</div>';
  $output .= '<div class="rules_element_parameter">';
  $output .= drupal_render($form['parameter']);

  $output .= '<ul class="rules_element_list">';
  foreach (element_children($form['parameter']) as $parameter) {
    $output .= '<li class="rules_element_list">';
    $output .= drupal_render($form['parameter'][$parameter]);
    $output .= '</li>';
  }
  $output .= '</ul>';

  $output .= '</div>';
  $output .= '<div class="rules_element_child">';
  foreach (element_children($form['parameter']) as $childs) {
    $output .= drupal_render($form['childs']);
  }
  $output .= '</div>';
  $output .= '</div>';

  return $output;
}
