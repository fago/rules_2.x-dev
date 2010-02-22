<?php
// $Id$

/**
 * @file Contains ui functions for elements like actions and conditions.
 */

interface RulesElementsUI extends RulesPluginUiInterface  {
  public function getName();
  public function getLabel();
  public function getParameters(&$form);
  public function getChilds(&$form);
}

class RulesElementsImplUI extends FacesExtender {
  public function implementsFaces() {
    return array('RulesElementsUI');
  }

  public function form(&$form, &$form_state, $options = array(), $element) {
    $form = array();
    $form['#theme'] = 'rules_elements';
    $form['head'] = array(
      '#markup' => $element->getName(),
    );

    $form['operator']['edit'] = array(
      '#markup' => 'edit',
    );
    $form['operator']['delete'] = array(
      '#markup' => 'delete',
    );

    $form['parameter'] = array(
      '#markup' => t('Parameter:'),
    );
    $element->__call('getParameters', array(&$form['parameter']));

    $form['childs'] = array();
    $element->__call('getChilds', array(&$form['childs']));
  }

  public function getName($element){
    return $element->getElementName();
  }

  public function getParameters(&$form, $element) {
    foreach($element->parameterInfo() as $name => $parameter) {
      $form[$name] = array(
        '#markup' => $name,
      );
    }
  }

  public function getChilds(&$form, $element) {}

}

class RulesConditionUI extends RulesElementsImplUI {
  public function getChilds(&$form, $condition) {}
}

class RulesActionUI extends RulesElementsImplUI {
  public function getChilds(&$form, $action) {
    $form['test'] = array(
      '#markup' => 'test',
    );
  }
}