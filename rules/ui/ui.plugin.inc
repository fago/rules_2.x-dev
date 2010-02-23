<?php
// $Id$

/**
 * @file Contains ui functions for elements like actions and conditions.
 */

interface RulesPluginUI extends RulesPluginUiInterface  {
  public function getName();
  public function getLabel();
  public function getSettingOverview(&$form);
  public function getChilds(&$form, &$form_state);
}

class RulesPluginUIImpl extends FacesExtender {
  public function implementsFaces() {
    return array('RulesPluginUI');
  }

  public function form(&$form, &$form_state, $options = array(), $element) {
    $form = array();
    $form['#theme'] = 'rules_plugins';
    $form['head'] = array(
      '#markup' => $element->getName(),
    );

    $form['operator'] = $element->operations();

    $form['settings'] = array();
    $element->__call('getSettingOverview', array(&$form['settings']));

    $form['childs'] = array();
    $element->__call('getChilds', array(&$form['childs'], &$form_state));
  }

  public function getName($element){
    return $element->getElementName();
  }

  public function getSettingOverview(&$form, $element) {
    $form['#markup'] = t('Parameter:');
    foreach($element->parameterInfo() as $name => $parameter) {
      $form[$name] = array(
        '#markup' => $name,
      );
    }
  }

  public function getChilds(&$form, &$form_state, $element) {}

  public function operations() {
    $form = array();
    $form['edit'] = array(
      '#markup' => 'edit',
    );
    $form['delete'] = array(
      '#markup' => 'delete',
    );
    return $form;
  }

}

class RulesConditionUI extends RulesPluginUIImpl {
}

class RulesActionUI extends RulesPluginUIImpl {
}

class RulesContainerUI extends RulesPluginUIImpl {
  public function getName() {
    return 'Loop';
  }

  public function getChilds(&$form, &$form_state, $loop) {
    foreach ($loop as $id => $children) {
      $children->form($form[$id], $form_state);
    }
  }

  public function operations() {
    $form = array();
    $form['edit'] = array(
      '#markup' => 'edit',
    );
    $form['delete'] = array(
      '#markup' => 'delete',
    );
    $form['add_or'] = array(
      '#markup' => '+ Add or',
    );
    $form['add_and'] = array(
      '#markup' => '+ Add and',
    );
    $form['add_loop'] = array(
      '#markup' => '+ Add Loop',
    );

    return $form;
  }
}