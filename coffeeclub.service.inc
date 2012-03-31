<?php

function coffeeclub_service_form(){
   global $user;
  
  $product_registraitions = coffeeclub_user_product_registraitions_load($user); // in coffeeclub.data.inc

  if( count($product_registraitions) < 1 ) { 
     $form['no_records'] = array(
      '#type' => 'hidden',
      '#prefix' => '<br /><br />You have not registered a product with us yet. To do so please visit '.l('Register a product','thecoffeeclub/address'),
      '#suffix' => '<br /><br /><br /><br /><br /><br />',
     );
     return $form;
  }
  
  $product_list = array();
  foreach($product_registraitions as $item){ $product_list[$item->rid."|".$item->model] = $item->model." - ID ".$item->serial; }
  $pkeys = array_keys($product_list);
  $default_product = $pkeys[0];
  
  if( isset($_SESSION['enquiry']['model'])){ $default_product = $_SESSION['enquiry']['model']; } 
   
  $form['product'] = array(
    //'#title' => t('Product'),
    '#type' => 'select',
    '#description' => t(''),
    '#options' => $product_list,
    '#default_value' => $default_product,
    '#size' => 6,
  );
  
  if( count(arg()) != 2 ){ return $form; }
  
  /*$form['product_edit'] = array(
    '#type' => 'submit',
    '#value' => t('Edit product information'),
    '#suffix' => '<br />',
  );*/
      
  $form['product_add'] = array(
    '#type' => 'submit',
    '#value' => t('Add another product'),
    '#suffix' => '<br />',   
  );
     
  $form['product_label_print'] = array(
    '#type' => 'submit',
    '#value' => t('Print service label'),
    '#suffix' => '<br />',    
  );
    
  $form['product_confirm'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm Product'),
  );
  
  
  return $form;

}

/**
* Validate the form.
*/
function coffeeclub_service_form_validate($form, &$form_state) {

}

/**
* form submission.
*/
function coffeeclub_service_form_submit($form, &$form_state) {

  $_SESSION['enquiry']['model'] = $form_state['values']['product'];

  $submit_value = $form_state['clicked_button']['#value'];
  switch( $submit_value ){
    case 'Add another product': $form_state['redirect'] = array('thecoffeeclub/address/product','destination=thecoffeeclub/service'); return;
    case 'Edit product information': ; break;    
    case 'Print service label': ; break;    
 
  }

  $form_state['redirect'] = 'thecoffeeclub/service/enquiry';

}



/**
* Enquiry type form
*/
function coffeeclub_enquiry_type_form() {
  $args = arg();
  
  $vid = variable_get('coffeeclub_service_enquiry_types_vid',1);
  $service_enquiry_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  $service_enquiry_list = array();
  foreach( $service_enquiry_taxonomy as $item) { $service_enquiry_list[$item->tid."|".$item->name] = $item->name; }
  $skeys = array_keys($service_enquiry_list);
  $default_service_enquiry = $skeys[0];
  
  if( isset($_SESSION['enquiry']['type'])){ $default_service_enquiry = $_SESSION['enquiry']['type']; } 

  $form['enquiry_type'] = array(
    //'#title' => t('Type of enquiry'),
    '#type' => 'select',
    '#description' => t(''),
    '#options' => $service_enquiry_list,
    '#default_value' => $default_service_enquiry,
    '#size' => 6,
  );

  if( $args[2] != 'enquiry' ){ return $form; }
  
    $form['enquiry_type_list_submit'] = array(
      '#type' => 'submit',
      '#value' => t('View current enquiries'),
      '#suffix' => '<br />',
    );   
      
    $form['enquiry_type_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Next'),
    );
 
  
  return $form;

}


/**
* Validate the form.
*/
function coffeeclub_enquiry_type_form_validate($form, &$form_state) {

}

/**
* form submission.
*/
function coffeeclub_enquiry_type_form_submit($form, &$form_state) {
  
  $enquiry_type_code = $form_state['values']['enquiry_type'];
  $_SESSION['enquiry']['type'] = $enquiry_type_code;
  
  if ( $form_state['clicked_button']['#value'] == 'Next' ) {
    $form_state['redirect'] = 'thecoffeeclub/service/message';
  // view enquiry list
  } else {
      $form_state['redirect'] = 'thecoffeeclub/service/enquiries';
  }



}

/**
* Enquiry type form
*/
function coffeeclub_enquiry_message_form() {
  $args = arg();
  
  $form['enquiry_message'] = array(
    '#title' => t(''),
    '#type' => 'textarea',
    '#default_value' => "Type here",
    '#rows' => 6,
    '#cols' => 20,
    '#description' => t(''),    
  );
  
  if( $args[2] != 'message' ){ return $form; }
  
    $form['enquiry_message_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Send enquiry'),
    );  
    
    $form['enquiry_message_alternate'] = array(
      '#type' => 'submit',
      '#value' => t('Add alternate address'),
    );

    
  return $form;

}


/**
* Validate the form.
*/
function coffeeclub_enquiry_message_form_validate($form, &$form_state) {



}



/**
* form submission.
*/
function coffeeclub_enquiry_message_form_submit($form, &$form_state) {
  global $user;
  
  $_SESSION['enquiry']['message'] = $form_state['values']['enquiry_message'];

  $submit_value = $form_state['clicked_button']['#value'];
  switch( $submit_value ){
    case 'Add alternate address': $form_state['redirect'] = array('thecoffeeclub/alternate','destination=thecoffeeclub/service'); return;

  }
  
  // check to see if this is a customer or service dept enquiry
  $type = explode("|",$_SESSION['enquiry']['type']);
  $is_customer_type = _coffeeclub_is_customer_service_type($type[1]); // in coffeeclub.data.inc
  
  coffeeclub_insert_product_enquiry(); // in coffeeclub.data.inc expects to recieve session object
  
  // trigger any relevant emails
  $model_data = explode("|",$_SESSION['enquiry']['model']);
  $registraition = coffeeclub_user_product_registraition_load($model_data[0]);
  
  $user->model = $registraition->model;
  $user->serial = $registraition->serial; 
  $user->is_customer_services = $is_customer_type; 
  module_invoke_all('coffeeclub', 'member_enquiry_create', $user);
  
  // send them off to the thank you page
  $form_state['redirect'] = 'thecoffeeclub/enquiry-thankyou';   
  
}


/**
* enquiries list form
*/
function coffeeclub_enquiry_list_form() {

  $vid = variable_get('coffeeclub_service_enquiry_status_vid',1);
  $service_enquiry_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  $service_enquiry_list = array();
  foreach( $service_enquiry_taxonomy as $item) { $service_enquiry_list[$item->tid] = $item->name; }
  
  $vid = variable_get('coffeeclub_service_enquiry_types_vid',1);
  $service_enquiry_type_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  $service_enquiry_type_list = array();
  foreach( $service_enquiry_type_taxonomy as $item) { $service_enquiry_type_list[$item->tid] = $item->name; }


  $model = explode("|", $_SESSION['enquiry']['model']);
  $rid = $model[0];

  $e_type = explode("|",$_SESSION['enquiry']['type']);
  $e_type_tid = $e_type[0];  
  $enquiries = coffeeclub_enquiries_load_by_current_user();
  
  
  $enquiry_options = array();
  $message_list = array();
  $reply_list = array();
  $status_list = array();
  $enquiry_type_list = array();
  if(count($enquiries) > 0 ){
    foreach( $enquiries as $enquiry ){
       if( $enquiry->rid == $rid ){ // && $enquiry->enquiry_type_tid == $e_type_tid
         $enquiry_options[$enquiry->eid] =  "Enquiry ".$enquiry->eid;
         $message_list[$enquiry->eid] = $enquiry->customer_message;
         $reply_list[$enquiry->eid] = $enquiry->service_message;
         $status_list[$enquiry->eid] = $service_enquiry_list[$enquiry->status_tid];
         $enquiry_type_list[$enquiry->eid] = $service_enquiry_type_list[$enquiry->enquiry_type_tid];
       }
       
    }
  }
  
  
  // no enquiries for this user, by this product and this enquiry type
  if( count($enquiry_options) <1 ){
    $form['no_enquiry_results'] = array(
      '#type' => 'markup',
      '#value' => 'You have no current '.$e_type[1].' enquiries about '.$model[1].'.',    
    );
    
  $form['enquiry_message_list_back'] = array(
    '#type' => 'submit',
    '#value' => t('Back'),
  );
  
    return $form;  
  }
  
  $messages = '<div class="message" ><ul>';  
  foreach( $message_list as $eid => $message ){
    $messages .= '<li id="'.$eid.'" >Your message: '.$message.'</li>';
  }
  $messages .= '</ul></div>';   
  
  $replies = '<div class="replies" ><ul>';  
  foreach( $reply_list as $eid => $reply ){
    $replies .= '<li id="'.$eid.'" >Services Reply: '.$reply.'</li>';
  }
  $replies .= '</ul></div>';
  
  $statuses = '<div class="enquiry-status" ><ul>';  
  foreach( $status_list as $eid => $status ){
    $statuses .= '<li id="'.$eid.'" >Current status: '.$status.'</li>';
  }
  $statuses .= '</ul></div>';
     
  $etypes = '<div class="enquiry-status" ><ul>';  
  foreach( $enquiry_type_list as $eid => $etype ){
    $etypes .= '<li id="'.$eid.'" >Enquiry Type: '.$etype.'</li>';
  }
  $etypes .= '</ul></div>';

  $form['enquiry_message'] = array(
    '#type' => 'select',
    '#options' => $enquiry_options,
    //'#default_value' => "Type here",
    '#size' => 3,
    '#description' => t(''),    
  );

  $form['enquiry_types'] = array(
    '#type' => 'markup',
    '#value' => $etypes,    
  );
  
  $form['enquiry_status'] = array(
    '#type' => 'markup',
    '#value' => $statuses,    
  );
    
  $form['enquiry_messages'] = array(
    '#type' => 'markup',
    '#value' => $messages,    
  );  
  
  $form['enquiry_replies'] = array(
    '#type' => 'markup',
    '#value' => $replies,    
  );
    
    
  $form['enquiry_message_message'] = array(
    '#type' => 'submit',
    '#value' => t('Send enquiry'),
  );  
  
  return $form;

}


/**
* Validate the form.
*/
function coffeeclub_enquiry_list_form_validate($form, &$form_state) {

}

/**
* form submission.
*/
function coffeeclub_enquiry_list_form_submit($form, &$form_state) {

  $submit_value = $form_state['clicked_button']['#value'];
  switch( $submit_value ){
    case 'Back': $form_state['redirect'] = 'thecoffeeclub/service/enquiry'; return;

  }
  
  $form_state['redirect'] = 'thecoffeeclub/service/message';
  
}


