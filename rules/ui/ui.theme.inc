<?php
// $Id$

/**
 * @file Rules theme functions
 */

/**
 * Themes the display of a configuration element.
 */
function theme_rules_element_view($variables) {
  $form = $variables['form'];

  //TODO: needs to be specified in the render array
  drupal_add_css(drupal_get_path('module', 'rules') . '/ui/rules.css');

  $output = '';
  $output .= '<div class="rules_plugin">';
  $output .= '<div class="rules_plugin_head">';
  $output .= drupal_render($form['head']);
  $output .= '</div>';


  $output .= '<div class="rules_plugin_operator">';
  foreach (element_children($form['operations']) as $operator) {
    $output .= '<div class="rules_plugin_operators">';
    $output .= drupal_render($form['operations'][$operator]);
    $output .= '</div>';

  }
  $output .= '</div>';

  $output .= '<div class="rules_plugin_parameter">';
  $output .= drupal_render($form['content']);
  $output .= '<ul class="rules_plugin_list">';
  foreach (element_children($form['content']) as $parameter) {
    $output .= '<li class="rules_plugin_list">';
    $output .= drupal_render($form['content'][$parameter]);
    $output .= '</li>';
  }
  $output .= '</ul>';
  $output .= '</div>';
  $output .= '<div class="rules_plugin_spacer">&nbsp;</div>';
  if (isset($form['childs']) && count(element_children($form['childs']))) {
    $output .= '<div class="rules_plugin_child">';
    foreach (element_children($form['childs']) as $childs) {
      $output .= drupal_render($form['childs']);
    }
    $output .= '</div>';
  }

  $output .= '</div>';

  return $output;
}

