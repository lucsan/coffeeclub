<?php

/**
 *  loads all product nodes 
 *
 */
function coffeeclub_products_load(){
  
  // model is the name of the product node
  
  $result = db_query("
  SELECT n.nid, tn.tid, n.type, n.title, n.status, td.vid, td.name 
  FROM {node} n 
  LEFT JOIN {term_node} tn
  ON  n.nid = tn.nid
  LEFT JOIN {term_data} td
  ON tn.tid = td.tid
  LEFT JOIN {content_type_product} ctp
  ON n.nid = ctp.nid
  WHERE n.type = 'product'
  AND n.status = 1
  AND ctp.field_product_registration_value = 'Include'
  ORDER BY tn.tid
  ");
  
  $rows = array();
  while( $row = db_fetch_object($result) ){ $rows[] = $row; }
  return $rows; // this is now an array of product details
}


/**
 *  load a product from its node id
 *  @params $pid - a product node id 
 *
 */
 function coffeeclub_product_load($pid){
 
    return db_fetch_object(db_query("
    SELECT 
    n.nid, n.title, n.status, ctp.field_product_price_value AS price, td.name 
    FROM {node} n
    LEFT JOIN {content_type_product} ctp
    ON n.nid = ctp.nid  
    LEFT JOIN {term_node} tn
    ON  n.nid = tn.nid
    LEFT JOIN {term_data} td
    ON tn.tid = td.tid 
    WHERE n.nid = %d;
    ",$pid));
 
 } 
 
 /**
  *
  * @param 
  * @return
  */
 function coffeeclub_load_annual_product_registrations($this_year, $this_month_date, $next_month_date){
    
    //reminder_sent (year flag)
    
    // 
    
    $rows = array();
    $query ="
    SELECT pr.rid, pr.product_nid, pr.uid, pr.registraition_date, pr.special_offers, pr.reminder_sent , pr.model, td.name, pr.serial  
    FROM {product_registrations} pr
    LEFT JOIN {term_node} tn
    ON pr.product_nid = tn.nid 
    LEFT JOIN {term_data} td
    ON tn.tid = td.tid
    WHERE  pr.reminder_sent <> %d
   
    AND pr.registraition_date <= %d
    AND td.name LIKE 'Bean to Cup'
    ";
/* 

        AND pr.registraition_date >= %d 
    WHERE pr.special_offers LIKE '%s'    
       
*/
    
//    LEFT JOIN {users} u ON pr.uid = u.uid    
         
    //$result = db_query($query,'%service%',$last_year_today);
    $result = db_query($query, $this_year, $this_month_date, $next_month_date);    
   
    /*dbg*/ //drupal_set_message('<pre> <br>'.$query.'</pre>');

    while( $row = db_fetch_object($result)) $rows[] = $row;
    

    return $rows;
    
 }
 
 /**
  * Update the reminder_sent field of product_regisraition with the current year
  * @param 
  * @return
  */
 function coffeeclub_annual_reminder_update($rid, $this_year){
    /*dbg*/ //drupal_set_message('<pre>call to update <br>'.$rid.'</pre>');
    /*dbg*/ //drupal_set_message('<pre> <br>'.print_r($this_year ,true).'</pre>');
    
    $result = db_query("UPDATE {product_registrations}
    SET reminder_sent = %d
    WHERE rid = %d
    ",$this_year,$rid );
    
 }
 
 
 function coffeeclub_annual_reminder_update_reset_test(){

    
    $result = db_query("UPDATE {product_registrations}
    SET reminder_sent = 2010
    WHERE rid >0
    ");
    
 } 
  

/**
 *  inserts a product registration line into the data base
 *  @param form_state['values'] - from coffeeclub_user_product_register_form in coffeeclub.user.inc 
 *  returns boolean
 */
function coffeeclub_insert_product_registration($form_values){
  global $user;
  if(!$user->uid) return false; // check user is loged in
  
  // reminder sent is marked with this year as no reminder is sent this year
  // so it is as if it has already been sent.
  $today = time();
  $this_year = date('Y',$today);
  
  // extract model and product nid 
  $temp = explode("|",$form_values['model']);
  $product_nid = $temp[0];
  $model = $temp[2];
  
  // format date to system encoded uk date (ie: - separated ) then convert to unix time stamp  
  $input_purchase_date = $form_values['purchase_date'];
  $uk_format_date = str_replace(array("/","."),"-",$input_purchase_date );
  $unix_purchase_date = strtotime($uk_format_date);
  
  // serialise the contact methods
  $contact = "";
  $form_values['contact_email'] == 1 ? $contact .= "email:" : $contact .= "";
  $form_values['contact_phone' ]== 1 ? $contact .= "phone:" : $contact .= "";  
  $form_values['contact_sms'] == 1 ? $contact .= "sms:" : $contact .= "";
  $form_values['contact_post'] == 1 ? $contact .= "post:" : $contact .= "";
  
  //serialise the special_offers
  $specials = '';
  $form_values['bean_offer'] == 1? $specials .= 'descaler:': $specials .= ''; 
  $form_values['service_offer'] == 1? $specials .= 'service:': $specials .= '';
    
  if(strlen($contact)>0) $contact = substr($contact,0,strlen($contact)-1); //remove trailing semi-colon
  
  $receipt = $form_values['receipt'];  
  
  return db_query("
  INSERT INTO {product_registrations}
  (uid, product_nid, model, serial, supplier_shop, purchase_date, registraition_date, 
  contact_methods, has_receipt, special_offers, reminder_sent )
  VALUES
  (%d, %d, '%s', '%s', '%s', %d, %d, '%s', %d, '%s', '%s' )
  ",
  $user->uid,
  $product_nid,
  $model,
  $form_values['product_id'],
  $form_values['supplier'],
  $unix_purchase_date,
  time(),
  $contact,
  $receipt,
  $specials,
  $this_year
  );
}


function coffeeclub_update_product_registration_special_offers($rid, $special_offers){
   return db_query("UPDATE {product_registrations}
   SET
   special_offers = '%s'
   WHERE rid=%d
   ",
   $special_offers,
   $rid);
}

/**
 * updates product registration from the user form
 *
 *
 */
function coffeeclub_update_product_registration($form_values){
   
  // format date to system encoded uk date (ie: - separated ) then convert to unix time stamp  
  $input_purchase_date = $form_values['purchase_date'];
  $uk_format_date = str_replace(array("/","."),"-",$input_purchase_date );
  $unix_purchase_date = strtotime($uk_format_date);
  
  // serialise the contact methods
  $contact = "";
  $form_values['contact_email']==1 ? $contact .= "email:" : $contact .= "";
  $form_values['contact_phone']==1 ? $contact .= "phone:" : $contact .= "";  
  $form_values['contact_sms']==1 ? $contact .= "sms:" : $contact .= "";
  $form_values['contact_post']==1 ? $contact .= "post:" : $contact .= "";
  
  if(strlen($contact)>0) $contact = substr($contact,0,strlen($contact)-1); //remove trailing semi-colon
  
   
   $rid = $form_values['model'];
   $serial = $form_values['product_id'];
   $supplier_shop = $form_values['supplier'];
   $form_values['receipt_options_address']==0? $has_receipt = 1: $has_receipt = 0;
   //$special_offers = $form_values['special_offers'];
   
   return db_query("UPDATE {product_registrations}
   SET
   serial = '%s',
   supplier_shop = '%s',
   purchase_date = %d,
   contact_methods = '%s',
   has_receipt = %d
   WHERE rid=%d
   ",
   $serial,
   $supplier_shop,
   $unix_purchase_date,
   $contact,
   $has_receipt,
   $rid);
}   

/**
 *  loads all the users' product registraitions
 *  returns a user product registration array
 */
 
 function coffeeclub_user_product_registraitions_load($user){
  if(!$user->uid) return array();
  
  /*$result = db_query("
  SELECT * FROM {product_registrations}
  WHERE uid = '%d'
  ", $user->uid);*/
  
  $result = db_query("  
  SELECT * FROM {product_registrations} pr
  LEFT JOIN {term_node} tn
  ON pr.product_nid = tn.nid
  LEFT JOIN {term_data} td
  ON tn.tid = td.tid
  WHERE pr.uid = %d 
  ORDER BY pr.registraition_date DESC ",
  $user->uid
  );
  
  $rows = array();
  while( $row = db_fetch_object($result) ){ $rows[] = $row; }
  return $rows; // this is now an array of users product registrations
  
} 

/**
 *  loads a product registraition
 *  returns a user product registration array
 *  @param $rid - a product registration id 
 */
 
 function coffeeclub_user_product_registraition_load($rid){
  return db_fetch_object(db_query("SELECT * FROM {product_registrations} pr WHERE pr.rid = %d ", $rid ));
}  


/**
 *  check model and serial agaist existing records
 *  @param - array of registraition records
 *  @param - array of form_sata['values']
 *  returns boolean  
 */
 
function coffeeclub_check_serial_duplication( $registraition_records, $new_serial ){
 
  if( count($registraition_records) < 1 ) return false;
 
  // extract model and product nid 
  //$temp = explode("|",$form_values['model']);
  //$product_nid = $temp[0];
  //$model = $temp[1];
  //$serial = $form_values['product_id'];
  
  // check this model - serial combo doesn't aready exist
  foreach( $registraition_records as $item ){
    if(  $item->serial == $new_serial ){ return true; }
  }
  
  return false;
 
}


/**
 * inserts an enquiry in product_srvice_enquiries
 * recieves $_SESSION['enquiry'] 
 *
 */  
function coffeeclub_insert_product_enquiry(){
  global $user;
  if(!$user->uid) return false; // check user is loged in
  
  // aquire the status list
  $vid = variable_get('coffeeclub_service_enquiry_status_vid',1);
  $service_enquiry_status = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);

  // the default status should be open or new
  $status_tid = 1;
  foreach( $service_enquiry_status as $state ){ if( strtolower($state->name) == 'open' || strtolower($state->name) == 'new' ){ $status_tid = $state->tid; } }
  
  // get the session passed values
  $model = explode("|",$_SESSION['enquiry']['model']);
  $rid = $model[0];
  $e_type_code = explode("|",$_SESSION['enquiry']['type']);
  $e_type_tid = $e_type_code[0];
  $message = htmlentities($_SESSION['enquiry']['message']);
  
  // insert into table
  return db_query("
    INSERT INTO {product_service_enquiries}
    (rid, enquiry_type_tid, status_tid, customer_message, service_message )
    VALUES
    ( %d, %d, %d, '%s', '%s' )",
    $rid,
    $e_type_tid,
    $status_tid,
    $message,
    ""  
  ); 
}


/**
 *  wrapper function for coffeeclub_enquiries_load_by_user($uid)
 *  loads current user 
 */
 function coffeeclub_enquiries_load_by_current_user(){
  global $user;
  if ($user->uid){ return coffeeclub_enquiries_load_by_user($user); }
 }

 
/**
 *  retrieves enquiry data for specified user
 *  @param user uid
 *  returns an enquiry object   
 *
 */
 function coffeeclub_enquiries_load_by_user($user){
  
  if( !$user->uid) return;
  
    $result = db_query("SELECT * FROM
    {product_registrations} pr
    JOIN {product_service_enquiries} pse
    ON pr.rid = pse.rid
    JOIN {term_data} td
    ON td.tid = pse.enquiry_type_tid
    AND pr.uid = %d", $user->uid );
    
    while( $row = db_fetch_object($result)) $rows[] = $row;
    

    return $rows;

 }   


/**
 *
 *
 */
 function coffeeclub_enquiries_load_all(){
 
 }
 
 
/**
 * update service message 
 * @params
 *  requires int enquiry id, string  
 */
 function coffeeclub_enquiry_service_message_update($eid, $message){
 
    return db_query("UPDATE {product_service_enquiries}
      SET service_message ='%s'
      WHERE eid = %d", $message, $eid);

 }
 
/**
 *
 *
 */
 function coffeeclub_enquiry_service_status_update($eid, $status_tid){
 
    return db_query("UPDATE {product_service_enquiries}
      SET status_tid = %d
      WHERE eid = %d", $status_tid, $eid);

 }
 
 
/**
 *
 *
 */
 function coffeeclub_taxonomy_lists($vid, $type = 'enquiry_status'){
  
  switch($type){
    case 'enquiry_status':   $vid = variable_get('coffeeclub_service_enquiry_status_vid',1); break;
  }
  
  $taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  
  $list = array();
  foreach ($taxonomy as $item) { $list[$item->tid] = $item->name; }
  
  return $list;

 }
 
 
 /**
  *  function to check if an enquiry type is a customer services type
  *
  *
  */
  function _coffeeclub_is_customer_service_type($service_type){
  $vid = variable_get('coffeeclub_customer_enquiry_types_vid',1);
  $customer_enquiry_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  foreach( $customer_enquiry_taxonomy as $item) { if( $item->name == $service_type ) return true; }
  return false;
}           