<?php
/*
 * This file contains transaction operations.
 */

/**
 * Class YAMoneyTransaction
 */
class YAMoneyTransaction extends stdClass {
  const STATUS_IN_PROCESS = 'in_process';
  const STATUS_PROCESSED = 'processed';
  const STATUS_PAYED = 'payed';
  const STATUS_FAILED = 'failed';

  /**
   * @var int Transaction id.
   */
  public $ymid;
  /**
   * @var int Transaction user id.
   */
  public $uid;
  /**
   * @var int Transaction amount.
   */
  public $amount;
  /**
   * @var int Unix timestamp when the transaction was created.
   */
  public $created;
  /**
   * @var string Transaction status.
   */
  public $status;
  /**
   * @var string Transaction user email.
   */
  public $mail;
  /**
   * @var int Order id.
   */
  public $order_id;
  /**
   * @var array Array of additional transaction information.
   */
  public $data;

  /**
   *
   */
  public function __construct() {
    $this->status = YAMoneyTransaction::STATUS_IN_PROCESS;
    $this->created = time();
    $this->data = array();
  }
}

/**
 * @param int $ymid Transaction id.
 * @return YAMoneyTransaction
 */
function yamoney_transaction_load($ymid) {
  $row = db_fetch_array(db_query('SELECT * FROM {yamoney_transaction} WHERE ymid = %d', $ymid));

  if (!$row) {
    return NULL;
  }

  $transaction = new YAMoneyTransaction();
  foreach ($row as $key => $value) {
    $transaction->{$key} = $value;
  }

  $transaction->data = unserialize($transaction->data);

  return $transaction;
}

/**
 * @param YAMoneyTransaction $transaction
 * @return bool
 */
function yamoney_transaction_save(YAMoneyTransaction $transaction) {
  $fields = (array) $transaction;
  $fields['data'] = serialize($transaction->data);

  return !!drupal_write_record('yamoney_transaction', $fields, $transaction->ymid ? 'ymid' : array());
}

/**
 * Simply set transaction status.
 * @param $ymid int Transaction id.
 * @param $status string Transaction status.
 * @return bool
 */
function yamoney_update_transaction_status($ymid, $status) {
  return !!db_query('UPDATE {yamoney_transaction} SET status = "%s" WHERE ymid = %d', $status, $ymid);
}

/**
 * Delete user payment transaction
 * @param int $ymid
 * @return bool
 */
function yamoney_transaction_delete($ymid) {
  return !!db_query('DELETE FROM {yamoney_transaction} WHERE ymid = %d', $ymid);
}

/**
 * Render all transactions page
 * @return mixed
 */
function yamoney_transactions_page() {
  $sql = 'SELECT * FROM {yamoney_transaction}';

  $rez = pager_query($sql, 20, 0, NULL);

  $transactions = array();

  while ($row = db_fetch_array($rez)) {
    $transactions[] = $row;
  }

  drupal_set_title(t('Transactions'));
  return yamoney_create_transaction_table($transactions);
}

/**
 * Render user transactions
 * @param stdClass $user
 * @return bool|string
 */
function yamoney_transactions_user_page($user) {
  $sql = 'SELECT * FROM {yamoney_transaction} WHERE uid = %d';

  $rez = pager_query($sql, 20, 0, NULL, $user->uid);

  $transactions = array();
  while ($row = db_fetch_array($rez)) {
    $transactions[] = $row;
  }

  drupal_set_title(t('My transactions'));
  return yamoney_create_transaction_table($transactions);
}


/**
 * @param array $table_rows
 * @return null|string
 */
function yamoney_create_transaction_table($table_rows) {
  $rows = array();
  if (count($table_rows) > 0) {
    foreach ($table_rows as $item) {
      $rows[] = yamoney_transaction_build_payment_row((array) $item);
    }
    $header = array(t('Id'), t('Name'), t('Amount'), t('Date/Time'), t('Status'), t('E-mail'), t('Order ID'));
    $output  = theme('table', $header, $rows);
    $output .= theme('pager', NULL, 20, 0);
  }
  else {
    $output = t('There no transactions.');
  }


  return $output;
}

/**
 * @param array $row
 * @return array
 */
function yamoney_transaction_build_payment_row($row) {
  unset($row['data']);
  foreach ($row as $key => $item) {
    if ($key == 'uid') {
      if ($item) {
        $user = user_load($item);
        $item = $user->name;
      }
      else {
        $item = 'guest';
      }
    }
    if ($key == 'created') {
      $item = format_date($item, 'custom', 'd-m-Y H:i:s');
    }
    if ($key == 'amount') {
      $item = $item . ' RUR';
    }
    $row[$key] = $item;
  }
  return $row;
}
