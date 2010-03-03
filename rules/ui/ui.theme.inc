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

/**
 * Themes a tree of rule elements in a draggable table.
 *
 * @ingroup themeable
 */
function theme_rules_elements($variables) {
  $form = $variables['element'];
  $form['#theme'] = 'table';
  $form['#header'] = array($form['#heading'], t('Weight'), t('Operations'));
  $form['#attributes']['id'] = drupal_html_id($form['#heading']);
  $form['#attached']['css'] = array(drupal_get_path('module', 'rules') . '/ui/rules.css');

  foreach (element_children($form) as $element_id) {
    $element = &$form[$element_id];

    // Add special classes to be used for tabledrag.js.
    $element['parent_id']['#attributes']['class'] = array('rules-parent-id');
    $element['element_id']['#attributes']['class'] = array('rules-element-id');
    $element['weight']['#attributes']['class'] = array('rules-element-weight');

    $row = array();
    $row[] = theme('indentation', array('size' => $element['#depth'])) . drupal_render($element['label']);
    $row[] = drupal_render($element['weight']) . drupal_render($element['parent_id']) . drupal_render($element['element_id']);
    $row[] = array('class' => 'rules-operations', 'data' => drupal_render($element['operations']));

    $row = array('data' => $row) + $element['#attributes'];
    $row['class'][] = 'draggable';
    if (!$element['#container']) {
      $row['class'][] = 'tabledrag-leaf';
    }
    $form['#rows'][] = $row;
  }
  if (!empty($form['#rows'])) {
    drupal_add_tabledrag($form['#attributes']['id'], 'match', 'parent', 'rules-parent-id', 'rules-parent-id', 'rules-element-id', TRUE, 10);
    drupal_add_tabledrag($form['#attributes']['id'], 'order', 'sibling', 'rules-element-weight');
  }
  return drupal_render($form);
}
