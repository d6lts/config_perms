<?php

namespace Drupal\config_perms\Access;

use Drupal\Core\Access\AccessResult;
use Drupal\Core\Entity\EntityTypeManagerInterface;
use Drupal\Core\Routing\Access\AccessInterface;
use Drupal\Core\Session\AccountInterface;

/**
 * Checks access for custom_perms routes.
 */
class ConfigPermsAccessCheck implements AccessInterface {

  /**
   * The entityTypeManager service.
   *
   * @var \Drupal\Core\Entity\EntityTypeManager
   */
  protected $entityTypeManager;

  /**
   * Constructor.
   *
   * @param \Drupal\Core\Session\AccountInterface $account
   *   Run access checks for this account.
   * @param \Drupal\Core\Entity\EntityTypeManagerInterface $entity_type_manager
   *   The Entity Type Manager.
   */
  public function __construct(AccountInterface $account, EntityTypeManagerInterface $entity_type_manager) {
    $this->entityTypeManager = $entity_type_manager;
  }

  /**
   * A custom access check.
   *
   * @param \Drupal\Core\Session\AccountInterface $account
   *   Run access checks for this account.
   *
   * @return \Drupal\Core\Access\AccessResult
   *   Determine if the user is allowed to access the path.
   */
  public function access(AccountInterface $account) {
    // @todo Get the request from the method signature once this is fixed
    // https://www.drupal.org/project/drupal/issues/2786941
    $request = \Drupal::request();
    $path = $request->getPathInfo();

    $custom_perms_storage = $this->entityTypeManager->getStorage('custom_perms_entity');
    $params = [
      'status' => TRUE,
      'path' => $path,
    ];
    $custom_perms = $custom_perms_storage->loadByProperties($params);
    // Sometimes the path is saved without the first slash so let's try again
    // without the initial slash.
    if (empty($custom_perms)) {
      $params['path'] = substr($params['path'], 1);
      $custom_perms = $custom_perms_storage->loadByProperties($params);
    }

    $access_result = AccessResult::allowed();

    // If it is still empty, then the URL is different and we don't have any
    // reason to block the access, so let's return allowed.
    // (There are other access checkers which still could block the access, so
    // even returning allowed here doesn't means that the user will be able to
    // access).
    if (empty($custom_perms)) {
      return $access_result;
    }

    foreach ($custom_perms as $custom_perm) {
      $access_result = AccessResult::allowedIf($account->hasPermission($custom_perm->label()));
    }

    return $access_result;
  }

}
