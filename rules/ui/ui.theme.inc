<?php
// $Id$

/**
 * @file Rules theme functions
 */



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

