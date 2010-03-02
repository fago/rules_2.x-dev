<?php
// $Id$

/**
 * @file Contains ui functions for elements like actions and conditions.
 */

class RulesConditionUI extends RulesPluginUI {
  public function operations($element) {
    $form = array();

    $config_name = $element->root()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RulesPluginUI::path($config_name, $element_id)),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RulesPluginUI::path($config_name, $element_id, 'delete')),
    );
    return $form;
  }
}

class RulesActionUI extends RulesPluginUI {
  public function operations($element) {
    $form = array();

    $config_name = $element->root()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RulesPluginUI::path($config_name, $element_id)),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RulesPluginUI::path($config_name, $element_id, 'delete')),
    );
    return $form;
  }
}

