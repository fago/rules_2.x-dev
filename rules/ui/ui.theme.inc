<?php
// $Id$

/**
 * @file Rules theme functions
 */



function theme_rules_plugins($variables) {
  $form = $variables['form'];

  drupal_add_css(drupal_get_path('module', 'rules') . '/ui/rules.css');

  $output = '';
  $output .= '<div class="rules_plugin">';
  $output .= '<div class="rules_plugin_head">';
  $output .= drupal_render($form['head']);
  $output .= '</div>';


  $output .= '<div class="rules_plugin_operator">';
  foreach (element_children($form['operator']) as $operator) {
    $output .= '<div class="rules_plugin_operators">';
    $output .= drupal_render($form['operator'][$operator]);
    $output .= '</div>';

  }
  $output .= '</div>';

  $output .= '<div class="rules_plugin_parameter">';
  $output .= drupal_render($form['settings']);
  $output .= '<ul class="rules_plugin_list">';
  foreach (element_children($form['settings']) as $parameter) {
    $output .= '<li class="rules_plugin_list">';
    $output .= drupal_render($form['settings'][$parameter]);
    $output .= '</li>';
  }
  $output .= '</ul>';
  $output .= '</div>';
  $output .= '<div style="clear: both;">&nbsp;</div>';
  if (count(element_children($form['childs']))) {
    $output .= '<div class="rules_plugin_child">';
    foreach (element_children($form['childs']) as $childs) {
      $output .= drupal_render($form['childs']);
    }
    $output .= '</div>';
  }

  $output .= '</div>';

  return $output;
}

