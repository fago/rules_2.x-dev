<?php
// $Id$

/**
 * @file Contains ui functions for elements like actions and conditions.
 */

class RulesConditionUI extends RulesPluginUI {
  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_UI_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/condition'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_UI_RULE_PATH . '/' . $rule . '/delete/' . $element_id  . '/condition'),
    );
    return $form;
  }
}

class RulesActionUI extends RulesPluginUI {
  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_UI_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/action'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_UI_RULE_PATH . '/' . $rule . '/delete/' . $element_id  . '/action'),
    );
    return $form;
  }
}

