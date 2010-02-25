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
      //TODO: Remove child element id.
      '#markup' => $element->getName() . '-' . self::childElementId($element),
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


  /**
   * Returns the child element associated with the given id.
   */
  public function getChildElementById(RulesPlugin $rules_config, $id) {
    self::indexRule($rules_config);
    if (isset($rules_config->map[$id])) {
      return $rules_config->map[$id];
    }
    return FALSE;
  }

  /**
   * Returns the child element id of the given child.
   */
  public function childElementId(RulesPlugin $element) {
    self::indexRule($element);
    return $element->childElementId;
  }

  /**
   * Generates child element ids for lookup up elements.
   */
  protected function indexRule(RulesPlugin $element, &$map = array()) {
    // Generate them only once.
    if (!isset($element->childElementId)) {
      $counter = &drupal_static(__FUNCTION__);
      if (!isset($counter)) {
        $counter = 0;
      }

      if (!$counter) {
        // Get the rules config.
        while ($parent = $element->property('parent')) {
          $element = $parent;
        }
        $element->map = array();
        $map =& $element->map;
      }
      $counter++;
      if (is_array($map)){
        $map[$counter] = $element;
      }
      $element->childElementId = $counter;

      if ($element instanceof RulesContainerPlugin) {
        foreach ($element as $child) {
          self::indexRule($child, $counter, $map);
        }
      }
    }
  }

}

class RulesConditionUI extends RulesPluginUIImpl {
  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/condition'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/' . $element_id  . '/condition'),
    );
    return $form;
  }
}

class RulesActionUI extends RulesPluginUIImpl {
  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/action'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/' . $element_id  . '/action'),
    );
    return $form;
  }
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

  public function operations($element) {
    $form = array();

    $rule = $element->getRootElement()->name;
    $element_id = self::childElementId($element);

    $form['edit'] = array(
      '#markup' => l(t('Edit'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/edit/' . $element_id . '/container'),
    );
    $form['delete'] = array(
      '#markup' => l(t('Delete'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/' . $element_id . '/container'),
    );

    $form['add_or'] = array(
      '#markup' => l(t('+ Add or'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/add/or'),
    );
    $form['add_and'] = array(
      '#markup' => l(t('+ Add and'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/add/and'),
    );
    $form['add_loop'] = array(
      '#markup' => l(t('+ Add Loop'), RULES_ADMIN_RULE_PATH . '/' . $rule . '/delete/loop'),
    );
    return $form;
  }
}

