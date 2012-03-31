<?php

/**
 *
 *
 */
function coffeeclub_user_login_form(){
  $drupal_login_form = user_login_block();
  
  unset($drupal_login_form['links']);
  
  $drupal_login_form['links'] = array(
    '#type' => 'markup',
    '#value' => '
      <li>'.l('Not a member','thecoffeeclub/membership').'</li>
      <li>'.l('Not a member and want to register a product','thecoffeeclub/address/product').'</li>
    ',
    '#weight' => 4,
  );
  
  $drupal_login_form['reset'] = array(
    '#type' => 'markup',
    '#value' => '
    <li><a id="password" href="#" >Forgotten your password?</a></li>
    ',
    '#weight' => 1,
  );  
  
  $drupal_login_form['submit']['#weight'] = 2;  
  
  // send user to welcome page
  $drupal_login_form['#redirect'] = 'thecoffeeclub';
  $drupal_login_form['#action'] = base_path().'thecoffeeclub/login?destination=thecoffeeclub';

  return $drupal_login_form; 
}

function coffeeclub_user_password_change_form(){
  
    $form['password_title'] = array (
       '#type' => 'markup',
       '#value' => '<div>Change your password</div>',       
    );
    
    $form['password_change'] = array (
      '#type' => 'textfield',
      '#title' => 'new password',
      //'#value' => 'test@localhost.localdomain',
      //'#weight' => 0,
      '#size' => 30,
    );
    
    $form['password_check'] = array (
      '#type' => 'textfield',
      '#title' => 'repeat password',
      //'#value' => 'test@localhost.localdomain',
      //'#weight' => 0,
      '#size' => 30,
    );
        
    $form['password_change_submit'] = array (
      '#type' => 'submit',
      '#value' => 'Change password',
      //'#weight' => 0,
    );
    
    return $form;       
}

function coffeeclub_user_password_change_form_validate($form, &$form_state){
    $values = $form_state['values'];
    
    if($values['password_change'] != $values['password_check'] ) {form_set_error('password','Emails do not match');  return false;}
    if($values['password_change'] == "" ) {form_set_error('password','No password entered');  return false;}    
    
    
    
}

function coffeeclub_user_password_change_form_submit($form, &$form_state){
    global $user;
    $values = $form_state['values'];
    if(!$user->uid){ return; } // no user to update
    
    $new_pass = $values['password_change'];
    $hash_pass = md5($new_pass);
    
    db_query("UPDATE {users} SET pass = '%s' WHERE uid = %d ",$hash_pass, $user->uid);
    
    drupal_set_message('Your password has been updated.');
    
}

function coffeeclub_user_password_retrieval_form(){

    $form['password_retrieval'] = array (
      '#prefix' => '<div id="forgot-title" >Forgot Password?</div>'.
      '<div id="not-recognised" >Email address not recognised</div>'.
      '<div id="email_title" class="email_title" >Registered Email</div>',
      '#type' => 'textfield',
      //'#value' => 'test@localhost.localdomain',
      '#weight' => 0,
      '#suffix' => '<div><a id="pwd_cancel" href="#" >cancel</a></div>',
    );
   
    $form['password_retrieval_submit'] = array (
      '#type' => 'submit',
      '#value' => 'submit',
      //'#weight' => 0,
    );
    
    /*$form['password_retrieval_akw'] = array (
      '#type' => 'markup',
      '#value' => '<div>password acknowledged</div>',
      //'#weight' => 0,
    );*/     
    
    return $form;    
    

}
  
function coffeeclub_user_password_retrieval_form_submit($form, &$form_state){
  $form_values = $form_state['values'];
  $email = $form_values['password_retrieval'];
  
// check email address is known
  $user_record = db_fetch_object(db_query("SELECT * FROM {users} WHERE mail = '%s' ",$email));
  
  if( !$user_record || !$user_record->uid ){  // user found
    $form_state['redirect'] = array('thecoffeeclub/login','pwr=ukw');
    return;
  }

// generate new password
// 65-90 97-122 48-57
  $seed = "";
  for($i=0; $i<2; $i++){
    $seed .= chr(rand(97,122)); // small letter
    $seed .= chr(rand(65,90)); // caps letter
    $seed .= chr(rand(48,57)); // number       
  }

  $time = time();

  $hash_seed = md5($seed);
  
  // update user record
  
  $sql = ("UPDATE {users} SET pass = '".$hash_seed."' WHERE uid = ".$user_record->uid);
  db_query($sql); 

// // email new password
  $user = user_load($user_record->uid);
  $user->uid = null;
  $user->new_password = $seed;
  
  module_invoke_all('coffeeclub', 'password_reminder', $user);
  

// acknowlegded

    $form_state['redirect'] = array('thecoffeeclub/login','pwr=akw');
  


}

/*
/usr/bin/php5 /home/sites/everydaythings.co.uk/public_html/cron.php
usr/bin/wget  http://seriousaboutcoffee.com.com/cron.php

usr/bin/php5 /usr/local/psa/seriousaboutcoffee.com/htmldocs/cron.php


                /var/www/vhosts/seriousaboutcoffee.com/htmldocs/cron.php

*/


/**
 * add/edit product form
 *
 */  
function coffeeclub_user_product_register_form(){
  global $user;
$args = arg();

if( !$user->uid ){  }

// select a purchase location form the taxonomy
  $vid = variable_get('coffeeclub_service_suppliers_vid',7);
  
  //$services_list = taxonomy_get_children($tid, $vid = 0, $key = 'tid');
  $suppliers_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  
  $suppliers_list = array();
  $suppliers_list[''] = 'Please choose';
  foreach($suppliers_taxonomy as $item){
    $suppliers_list[$item->name] = $item->name;
  }
  
  // get available models
  $products_list = coffeeclub_products_load(); // in coffeeclub.data.inc

  $product_options = array();
  $product_types = array();  
  $product_options[''] = 'Please choose';
  foreach( $products_list as $item ){ 
    $product_options[$item->nid.'|'.$item->tid.'|'.$item->title] = $item->title;
    //$product_types 
  }

  $date = date("d/m/Y",time());

  
  if(isset($_GET['destination'])){
    $form['destination'] = array(
      '#type' => 'hidden',
      '#value' => $_GET['destination'],
    );    
  }
 
  
  $form['model'] = array(
    '#title' => 'Model',
    '#type' => 'select',
    '#options' => $product_options,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#required' => true,
    //'#description' => t('.'),
  );
  

  $form['supplier'] = array(
    '#title' => 'Where was it purchased?',
    '#type' => 'select',
    '#options' => $suppliers_list,
    '#default_value' => t('Please select'),    
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>
    ',
    //'#required' => true,
    //'#description' => t('Please select from the list.'),
  );
  
  
  // product id - bean to cup
  $form['product_id_1'] = array(
    '#type' => 'textfield',
    '#title' => 'Product ID number',
    '#size' => 1,
    //'#required' => true,  
    '#prefix' => t('
    <div id="product-id" name="product-id" >
    <div class="serial product-id" >
    <div class="container-inline container"><br />'),
    );
    
    
  
  for($i=2;$i<19;$i++){
  $form['product_id_'.$i] = array(
    '#type' => 'textfield',
    '#size' => 1,
    //'#required' => true,
  );   
  }
  
  $form['product_id_19'] = array(
    '#type' => 'textfield',
    '#size' => 1,
    //'#required' => true,
    '#description' => t('<div class="example" title="click to see example" >?</div><br />Product ID can be found on the machine behind the water tank.<br />'),  
    '#suffix' => t('</div></div></div><!-- product id -->'),
    );  
  
  
    
  /*$form['product_id'] = array(
    '#title' => 'Product ID number',
    '#type' => 'textfield',
    '#size' => 30,
    '#description' => t('<div class="example" >?</div><br />The product ID can be found either on the base of your machine or behind the water tank, dependent on your machine.'),
    '#prefix' => '<div class="serial product-id" ><div class="container-inline container">',
    '#suffix' => '</div></div>',
    //'#required' => true,
  );*/ 
  


    
  $form['product_serial'] = array(
    '#title' => 'Product serial number',
    '#type' => 'textfield',
    '#size' => 30,
    '#description' => t('<div class="example" title="click to see example" >?</div><br />Product serial number can be found on the base of the machine.'),
    '#prefix' => '<div id="product-serial" class="serial product-id" ><div class="container-inline container">',
    '#suffix' => '</div></div><!-- product serial-->',
    //'#required' => true,
  );
  
  
      
  $form['purchase_date'] = array(
    '#title' => 'Date of purchase',
    '#type' => 'textfield',
    '#default_value' => $date,
    '#description' => t('In the form dd/mm/yyyy.'),
    '#size' => 9,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#required' => true,
  );

  
  if( $args[1] == 'membership'){
  
    $receipt_options = array(
      t('I don\'t have a De\'Longhi product and would just like to receive newsletters.'),
      t('I have a valid receipt/proof of purchase. In the event of a repair it will be required to validate your guarantee.'),
      t('I no longer have a valid receipt/proof of purchase. Please note this affect warrantee claims.'),      
    );
  
  
    $form['receipt_options_membership'] = array(
       //'#prefix' => '<div class="title" >I\'d like De\'Longhi to contact me:</div>',
      //'#title' => t(''),
      '#type' => 'radios',
      '#default_value' => 1,
      '#options' => $receipt_options,
      '#required' => true,
      //'#description' => t('.'),
    );
    
        
  }else{
  
    // set the required fields
    $form['purchase_date']['#required'] = true;   
    //$form['product_id']['#required'] = true;
    $form['model']['#required'] = true;
  
    $receipt_options = array(
      t('I have a valid receipt/proof of purchase. In the event of a repair it will be required to validate your guarantee.'),
      t('I no longer have a valid receipt/proof of purchase. Please note this affect warrantee claims.'),      
    );
  
  
    $form['receipt_options_address'] = array(
       //'#prefix' => '<div class="title" >I\'d like De\'Longhi to contact me:</div>',
      //'#title' => t(''),
      '#type' => 'radios',
      '#default_value' => 0,
      '#options' => $receipt_options,
      '#required' => true,
      //'#description' => t('.'),
    );  
  
    /*$form['contact_email'] = array(
       '#prefix' => '<div class="title" >I\'d like De\'Longhi to contact me:</div>',
      '#title' => t('via email'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );
    
    $form['contact_post'] = array(
      '#title' => t('by post'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );    
      
    $form['contact_sms'] = array(
      '#title' => t('via SMS'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );
    
    $form['contact_phone'] = array(
      '#title' => t('by phone'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );*/ 
     
  }
  
    $form['contact_email'] = array(
       '#prefix' => '<div class="title" >I\'d like De\'Longhi to contact me:</div>',
      '#title' => t('via email'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );
    
    $form['contact_post'] = array(
      '#title' => t('by post'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );    
      
    $form['contact_sms'] = array(
      '#title' => t('via SMS'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );
    
    $form['contact_phone'] = array(
      '#title' => t('by phone'),
      '#type' => 'checkbox',
      //'#description' => t('.'),
    );
    
    $form['product_submit'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
      '#weight' => 1,
    );
  
  
  // only if we have membership/something (usually product)  
  if( $args[1] == 'membership' && !isset($args[2]) ){ 
    unset($form['product_submit']);
  }
  
  if(!$user->uid && !isset($_SESSION['address']['first_name'])){
    unset($form['product_submit']);
  }
  
  if( $user->uid && $_SESSION['process']!= 'adding product' ){
    unset($form['product_submit']);  
  }
    
  return $form;

}

/**
* Validate the form.
*/
function coffeeclub_user_product_register_form_validate($form, &$form_state) {
  global $user;
  

  // check to see if the new member has a machine 
  $receipt_option = $form_state['values']['receipt_options'];
    //if($receipt_option > 0){
    
    // check all fields have been selected/populated
     if( $form_state['values']['model'] == "" ){ form_set_error('model','Please select a model.');  }
    if( $form_state['values']['supplier'] < 0 ){ form_set_error('supplier','Please select a purchase location.');  }      
    //if( $form_state['values']['product_id'] == "" ){ form_set_error('product_id','Please enter a product ID number.'); $fail = true; }
    if( $form_state['values']['purchase_date'] == "" ){ form_set_error('purchase_date','Please enter a vaild date of purchase.');  }    

    
    if( $user->uid ){ 
      // check the user hasn't already registered this serial number
      $registraition_records = coffeeclub_user_product_registraitions_load($user);
      
      
    if( $form_state['values']['product_serial'] == ""){
      $number = "";
      for($i=1; $i<21; $i++){
         $number .=  $form_state['values']['product_id_'.$i];
      }
      $user->serial = $number;
    }else{
      $user->serial = $form_state['values']['product_serial'];
    }

      if( $user->serial == "" ){ form_set_error('product_serial','Please enter a product ID or serial number.'); }
      else{
        // tere is a number, check it
        if( coffeeclub_check_serial_duplication( $registraition_records, $user->serial ) ){ form_set_error('product_serial',t('This product has already been registered.')); }//in cofeeclub.data.inc
      }
     
      
    
     }
  //}
   
}

/**
* form submission.
*/
function coffeeclub_user_product_register_form_submit($form, &$form_state) {
  global $user;

  if ( isset($_SESSION['coffeeclub']['membership_route']) ){
    unset($_SESSION['coffeeclub']['membership_route']);
  }
  
  if(isset($_SESSION['process'])){ unset($_SESSION['process']);}
  
  
    // prepair receipt choice for db and message
    $receipt_option = 'intrest';
    
    if( isset($form_state['values']['receipt_options_membership'] )) {
      switch($form_state['values']['receipt_options_membership']){
        case(1): $receipt_option = 'have'; $form_state['values']['receipt'] = 1;  break;
        case(2): $receipt_option = 'have_not'; $form_state['values']['receipt'] = 0; break;        
      }
          
    // is receipt_options_address (only 2 options)
    }elseif( isset($form_state['values']['receipt_options_address'] )){
      switch($form_state['values']['receipt_options_address']){
        case(0): $receipt_option = 'have'; $form_state['values']['receipt'] = 1; break;
        case(1): $receipt_option = 'have_not'; $form_state['values']['receipt'] = 0; break;        
      }      
    }
    
    // prepair model for message
    $model_data = explode( "|", $form_state['values']['model'] );
    $model_nid = $model_data[0];
    $model_tid = $model_data[1];
    $model_ref = $model_data[2];
    
    $product = coffeeclub_product_load($model_nid); //in coffeeclub.data.inc
    
    // email values for both member and non-member product registration
    $user->product_class = $product->name;
    $user->model = $product->title;    
    
    // extract serial or number
    if( $form_state['values']['product_serial'] == ""){
      $number = "";
      for($i=1; $i<20; $i++){
         $number .=  $form_state['values']['product_id_'.$i];
      }
      $user->serial = $number;
    
    }else{
      $user->serial = $form_state['values']['product_serial'];
    }

    $form_state['values']['product_id'] = $user->serial; // pass the value back to the insert function

    //$user->serial = $form_state['values']['product_id'];
    
   
    // if the user does not have a product then they can't register one
    // if the user is not a member then no data can be inserted
    if ( $receipt_option != 'intrest' && $user->uid ){ // the user is a registered member
 
      // bean to cup special offer
      if($product->name == 'Bean to Cup'){ 
        $user->beantocupoffer = true; 
        $form_state['values']['bean_offer'] = true;
      }
      if($product->name == 'Bean to Cup' && $product->price > 799.99 ){
        $user->beantocupserviceoffer = true;
        $form_state['values']['service_offer'] = true; 
      }

      // insert the new record 
      $return = coffeeclub_insert_product_registration($form_state['values']); // in coffeeclub.data.inc
      if($return){ 
        // additional email values added to the user object
        $user->customer_address = $user->profile_houseno.
        "\n".$user->profile_street1.
        "\n".$user->profile_street2.
        "\n".$user->profile_county.
        "\n".$user->profile_postcode.
        "\n".$user->profile_number_daytime.
        "\n".$user->profile_number_mobile;
        
        // print out the special offer popup div if entitled to free service
        if( $user->beantocupserviceoffer){
          print ('<div id="special-offer-popup" class="special-offer" style="display:block;" >');
          //if( $user->beantocupoffer ) print('<div class="descaler" >You are entitled to a free descaler.</div>');
          if( $user->beantocupserviceoffer ) print('<div class="service" >
          Our customer experience team will contact you in the next 12 months to arrange your free annual service.
          </div>');        
          print ('</div>');
        }

      }
      
      // send email to service/customer dept to notify member has registerd a product
      module_invoke_all('coffeeclub', 'member_product_register', $user);
      
      
      // if not register another product then go somewhere else
      if( $form_state['clicked_button']['#value'] == 'Submit' ){
        $registrations = coffeeclub_user_product_registraitions_load($user);
        if(count($registrations) > 1){
          $form_state['redirect'] = 'thecoffeeclub/product-thankyou';
          watchdog('product registered','User '.$user->uid.' registered a product.', array(), WATCHDOG_NOTICE, l('view user','admin/user/thecoffeeclub/accounts/'.$user->uid.'/products'));
        }else{ // assume they have just become a member
          $form_state['redirect'] = 'thecoffeeclub/membership-thankyou';
        }
        
         
      }
  // get straight in there and thank them
  }else{
    // user is registering a product, but is not a member
    if ( !$user->uid ) {
      // collect service information on customers product
      $receipt_option = 0 ? $receipt = 'No proof of purchase' : $receipt = 'Holds proof of purchase' ;
      $contact_preferences = "";
      if( $form_state['values']['contact_email'] )  $contact_preferences  .= "email, ";
      if( $form_state['values']['contact_phone'] )  $contact_preferences  .= "phone, ";   
      if( $form_state['values']['contact_post'] )  $contact_preferences  .= "post, ";   
      if( $form_state['values']['contact_sms'] )  $contact_preferences  .= "sms, ";
      
      if ( $contact_preferences  != "" ){ $contact_preferences = substr( $contact_preferences, 0, strlen( $contact_preferences ) -2 ); }
      else { $contact_preferences = "No preference given"; }  
      
      // customer address details
      $customer_address = $_SESSION['address']['first_name']." ".$_SESSION['address']['last_name'].
      "\n".$_SESSION['address']['house_no'].
      "\n".$_SESSION['address']['street1'].
      "\n".$_SESSION['address']['street2'].
      "\n".$_SESSION['address']['county'].
      "\n".$_SESSION['address']['postcode'].
      "\nemail: ".$_SESSION['address']['mail'].
      "\nhome phone: ".$_SESSION['address']['phone'].
      "\nmobile phone: ".$_SESSION['address']['mobile'];
      
      $product_details = "model : ".$model_ref.
      "\npurchased at : ".$form_state['values']['supplier'].
      "\npurchased on : ".$form_state['values']['purchase_date'].
      "\nproduct serial : ".$user->serial.
      "\nproof of purchase: ".$receipt.
      "\ncontact preferences: ".$contact_preferences;
      
      // load email additional data to user object
      $user->mail = trim($_SESSION['address']['mail']);
      $user->profile_forename = $_SESSION['address']['first_name'];
      $user->profile_lastname = $_SESSION['user_details']['last_name'];
      $user->customer_address = $customer_address;
      $user->product_details = $product_details;
      $user->model = $model_ref;
      //$user->serial = $user->serial; 
     
    
      // call non-member (customer) product registration trigger
      // customer confirmation email
      module_invoke_all('coffeeclub', 'customer_product_register', $user);
      
      // (not required)
      // services notification email
      //module_invoke_all('coffeeclub', 'service_non_member_product_register_notification', $user);   
      
      $form_state['redirect'] = 'thecoffeeclub/non-member-thankyou';
      
    // user has no product, but is a member
    }else{ $form_state['redirect'] = 'thecoffeeclub/membership-thankyou'; } 
   
  }    
  
   //$name = $form_state['values']['user_name'];
  //drupal_set_message(t('Thanks for filling out the form, %name',  array('%name' => $name)));
  
}



/** add/edit address form
 *
 */  
function coffeeclub_user_address_register_form(){
  global $user;
  $args = arg();
  
  $page_origin = $_SESSION['origin']['page'];
  
  // submit button behaviour tracking.
  if(!isset($_SESSION['process'])){
     $_SESSION['process'] = 'checking details';
  }
  
  if( $user->uid && $user->uid > 0 ) {
    // ensure the profiles have been loaded to the user object 
    profile_load_profile($user);
    
    // ensure the users previous newsletter election stays
    
    $form['email_news'] = array (
      '#type' => 'hidden',
      '#value' => $user->profile_marketing_emails,
      '#weight' => 0,
    ); 
  
  }
  
  // we need to hold over the address details in a session object if the user is not logged on
  if ( $user->uid == 0 && isset($_SESSION['address']) ){
      $user->profile_forename = $_SESSION['address']['first_name'];
      $user->profile_surname = $_SESSION['address']['last_name'];
      $user->profile_houseno = $_SESSION['address']['house_no'];            
      $user->profile_street1 = $_SESSION['address']['street1'];      
      $user->profile_street2 = $_SESSION['address']['street2'];
      $user->profile_county = $_SESSION['address']['county'];      
      $user->profile_postcode = $_SESSION['address']['postcode'];
      $user->mail = $_SESSION['address']['mail'];
      $user->profile_number_daytime = $_SESSION['address']['phone'];      
      $user->profile_number_mobile = $_SESSION['address']['mobile'];      
      
  }
  

  $form['origin']['page'] = array(
    '#type' => 'hidden',
    '#value' => $page_origin,
  );
  
  $form['first_name'] = array(
    '#title' => t('First Name'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_forename,
    //'#description' => t('Please enter your first name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#weight' => -10,
  );
  
  $form['last_name'] = array(
    '#title' => t('Last Name'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_surname,    
    //'#description' => t('Please enter your last name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#weight' => -9,    
  );
  
  $form['house_no'] = array(
    '#title' => t('House name / number'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_houseno,    
    //'#description' => t('Please enter your house name or number.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#weight' => -8,     
  );
  
  if( !$user->uid ){ $form['house_no']['#required'] = true; }  
  
  $form['street1'] = array(
    '#title' => t('Street 1'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_street1,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#weight' => -7,     
  );
  
  if( !$user->uid ){ $form['street1']['#required'] = true; }  
  
  $form['street2'] = array(
    '#title' => t('Street 2'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_street2,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#weight' => -6,    
  );  
  
  
  $form['county'] = array(
    '#title' => t('County'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_county,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#weight' => -5,    
  );   
  
  $form['postcode'] = array(
    '#title' => t('Postcode'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_postcode,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#weight' => -4,     
  );
  
  if ( !$user->uid ){
    $form['mail'] = array(
      '#title' => t('Email'),
      '#type' => 'textfield',
      '#size' => 20,
      '#default_value' => $user->mail,
      '#prefix' => '<div class="container-inline container">', 
      '#suffix' => '</div>',    
      '#required' => true,
      //'#description' => t('.'),
    '#weight' => -3,       
    );    
    
    $form['mail1'] = array(
      '#title' => t('Confirm Email'),
      '#type' => 'textfield',
      '#size' => 20,
      '#default_value' => $user->mail,
      '#prefix' => '<div class="container-inline container">', 
      '#suffix' => '</div>',    
      '#required' => true,
    '#weight' => -3,      
      //'#description' => t('.'),
    );
     
  }
  
  $form['phone'] = array(
    '#title' => t('Daytime phone'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_number_daytime,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
  );  
  
  $form['mobile'] = array(
    '#title' => t('Mobile Number'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_number_mobile,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
  );
  
      
  $form['submit_address'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  
  if( $args[1] == 'account' ){
    unset($form['submit_address']);
    $form['aternate_address'] = array(
      '#type' => 'submit',
      '#value' => t('Add another address'),
    );  
  
    // move submit button below additional address button
    $form['submit_address'] = array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    );  
  }
  
  
  if( $args[1] == 'alternate' ){ $form['submit_address']['#value'] = t('Update address'); }
  
  if($_SESSION['process']!='checking details' && $args[1] != 'alternate'){
    unset($form['submit_address']);
  }   
 
  return $form;

}

/**
* Validate the form.
*/
function coffeeclub_user_address_register_form_validate($form, &$form_state) {

}

/**
* form submission.
*/
function coffeeclub_user_address_register_form_submit($form, &$form_state) {
  global $user;
  
  if( isset($_SESSION['coffeeclub']['membership_route']) &&  $_SESSION['coffeeclub']['membership_route'] == 'address' ){
    $_SESSION['coffeeclub']['membership_route'] = "product";
  }
  
  // check if user is a member
  if ( $user->uid && $user->uid != 0 ){
  
    // save or update user  Personal Details and Address 
    $edit = array(
      'profile_forename' => $form_state['values']['first_name'],
      'profile_surname' => $form_state['values']['last_name'],
    );
    profile_save_profile( $edit, $user, 'Personal Details' );
    
    $edit = array(
      'profile_houseno' => $form_state['values']['house_no'],
      'profile_street1' => $form_state['values']['street1'],
      'profile_street2' => $form_state['values']['street2'],
      'profile_county' => $form_state['values']['county'],
      'profile_postcode' => $form_state['values']['postcode'],
      'profile_number_daytime' => $form_state['values']['phone'],      
      'profile_number_mobile' => $form_state['values']['mobile'],

    );
    profile_save_profile( $edit, $user, 'Address' );
    
    $edit = array(
      'profile_marketing_emails' => $form_state['values']['email_news'],            
    );
    profile_save_profile( $edit, $user, 'Contact Settings' );    
  
  // user is not logged on save off address details to session object  
  }else{
  
    $_SESSION['address']['first_name'] = $form_state['values']['first_name'];
    $_SESSION['address']['last_name'] = $form_state['values']['last_name'];    
    $_SESSION['address']['house_no'] = $form_state['values']['house_no'];
    $_SESSION['address']['street1'] = $form_state['values']['street1'];    
    $_SESSION['address']['street2'] = $form_state['values']['street2'];    
    $_SESSION['address']['county'] = $form_state['values']['county'];
    $_SESSION['address']['postcode'] = $form_state['values']['postcode'];
    $_SESSION['address']['phone'] = $form_state['values']['phone'];    
    $_SESSION['address']['mobile'] = $form_state['values']['mobile'];        
    $_SESSION['address']['mail'] = $form_state['values']['mail'];        
  }
    
  $submit_value = $form_state['clicked_button']['#value'];

  $_SESSION['process'] = 'adding product';
  
     
  switch( $submit_value ){
    case 'new_member': 
    return;  
    case 'Update':
    case 'Submit':    
      $form_state['redirect'] = 'thecoffeeclub/address/product'; 
    return;
    case 'Add another address': 
      $form_state['redirect'] = 'thecoffeeclub/alternate'; 
    return;
    case 'Update address': 
      $form_state['redirect'] = 'thecoffeeclub/alternate'; 
    return;

  }
  
  // default to product regitration in the become a member workflow  
  $form_state['redirect'] = 'thecoffeeclub/address/product';
}


function coffeeclub_user_membership_form() {
  global $user;

  $user_details = array();
    $user_details['first_name'] = '';
    $user_details['last_name'] = '';    
    $user_details['mail'] = '';
    $user_details['password'] = '';
    $user_details['email_news'] = 1;
    
            
         
  if($user->uid){
    // call to profile_load_profile loads profile data into the user object
    profile_load_profile($user);
    $user_details['first_name'] = $user->profile_forename;
    $user_details['last_name'] = $user->profile_surname;    
    $user_details['mail'] = $user->mail;
    //$user_details['password'] = "******";
    $user_details['email_news'] = $user->profile_marketing_emails;            
  }
  
  $form['first_name'] = array(
    '#title' => t('First Name'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $user_details['first_name'],
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>', 
    '#required' => true,
    //'#description' => t('.'),
  );
  
  $form['last_name'] = array(
    '#title' => t('Last Name'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $user_details['last_name'],
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    '#required' => true,
    //'#description' => t('.'),
  );
  
  $form['mail'] = array(
    '#title' => t('Email'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $user_details['mail'],
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',    
    '#required' => true,
    //'#description' => t('.'),
  );    
  
  $form['mail1'] = array(
    '#title' => t('Confirm Email'),
    '#type' => 'textfield',
    '#size' => 20,
    '#default_value' => $user_details['mail'],
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',    
    '#required' => true,
    //'#description' => t('.'),
  );  
  
  $form['pass'] = array(
    //'#title' => t('Password'),
    '#type' => 'password_confirm',
    '#size' => 20,
    //'#default_value' => $user_details['password'],    
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',    
    '#required' => true,
    //'#description' => t('.'),
  );  
  
  /*$form['pass1'] = array(
    '#title' => t('Confirm Password'),
    '#type' => 'password',
    '#size' => 20,
    //'#default_value' => $user_details['password'],     
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',    
    '#required' => true,
    //'#description' => t('.'),
  );*/
  
  
  $form['house_no'] = array(
    '#title' => t('House name / number'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_houseno,    
    //'#description' => t('Please enter your house name or number.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#weight' => -8,     
  );
  
  if( !$user->uid ){ $form['house_no']['#required'] = true; }  
  
  $form['street1'] = array(
    '#title' => t('Street 1'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_street1,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#weight' => -7,     
  );
  
  if( !$user->uid ){ $form['street1']['#required'] = true; }  
  
  $form['street2'] = array(
    '#title' => t('Street 2'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_street2,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#weight' => -6,    
  );  
  
  
  $form['county'] = array(
    '#title' => t('County'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_county,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#weight' => -5,    
  );   
  
  $form['postcode'] = array(
    '#title' => t('Postcode'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_postcode,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#required' => true,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#weight' => -4,     
  );
    
  $form['phone'] = array(
    '#title' => t('Daytime phone'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_number_daytime,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
  );  
  
  $form['mobile'] = array(
    '#title' => t('Mobile Number'),
    '#type' => 'textfield',
    '#default_value' => $user->profile_number_mobile,    
    //'#description' => t('Please enter your name.'),
    '#size' => 20,
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
  );  
  
  
  
  
  
  
  
  $form['email_news'] = array(
    '#title' => t('I\'d like to receive emails from De\'Longhi.'),
    '#type' => 'checkbox',
    '#default_value' => $user_details['email_news'],
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',    
    //'#required' => true,
    //'#description' => t('.'),
  );
  
  $form['submit_membership'] = array(
    '#type' => 'submit',
    '#value' => t('next'),
   
    //'#required' => true,
    //'#description' => t('.'),
  );
  
  // the user has submitted the form
  if(count(arg()) > 2 ) unset($form['submit_membership']);    
 

  return $form;

}

function coffeeclub_user_membership_form_validate($form, &$form_state) {

  //if($form_state['values']['pass'] != $form_state['values']['pass1']) {form_set_error('pass','Passwords do not match');  return false;}
  if($form_state['values']['mail'] != $form_state['values']['mail1']) {form_set_error('mail','Emails do not match');  return false;}

}

function coffeeclub_user_membership_form_submit($form, &$form_state) {
  global $user;
  
  //profile save profile changes the form, removing the address details
  $orig_form_state = $form_state;

  $_SESSION['coffeeclub']['membership_route'] = 'product';

  
  $mail = $form_state['values']['mail'];
  $pass = $form_state['values']['pass'];
  $form_state['values']['name'] = $mail;
  // Pull profile values from the form_state as user_register modifies form_state and clears these values without saving them.
  $first_name = $form_state['values']['first_name'];
  $last_name = $form_state['values']['last_name'];
  $email_news = $form_state['values']['email_news'];  
  
  // send welcome email (invoke new member trigger)
  /** 
  * the call to module_invoke_all  should be after drupal_execute and should not require
  * the mail variable, however, the behaviour of the production site and my dev build are
  * different and on production the drupal_execute call is nulling the user values thus removing
  * the users profile names which are needed for the welcome email. So I have placed the email call first
  * meaning emails will be sent even if the user fails to register.
  */      
  
  $user->profile_forename = $first_name;
  $user->profile_surname = $last_name;
  $user->mail = $mail;  
  
  module_invoke_all('coffeeclub', 'member_welcome', $user);
  
  // submit new user registration details
  $form_state['values']['password'] = $pass;

  drupal_execute('user_register',$form_state);

  // check the status of user_email_verification setting
  if( variable_get('user_email_verification', TRUE) ){
    drupal_set_message("User verification enabled. Automatic logon disabled. An email with your logon details has been sent to your email account.");
  }

  // prepair the profile details extracted earlier
  $edit = array(
    'profile_forename' => $first_name,
    'profile_surname' => $last_name,
  );
      
  // send the profile details to be saved
  profile_save_profile($edit, $user, 'Personal Details' );
  
  // do the same for contact settings
  $edit = array(
    'profile_marketing_emails' => $email_news,
  );
  profile_save_profile($edit, $user, 'Contact Settings' );
  
  
  // now save the address details
  $orig_form_state['clicked_button']['#value'] = 'new_member';
  
  coffeeclub_user_address_register_form_submit($form, $orig_form_state);

  
  // return the workflow to the membership page /products (same page now ready for product details to be entered)
  $form_state['redirect'] = 'thecoffeeclub/membership/product';  

}


function coffeeclub_user_registered_products_list_form($form_state){
  global $user;
  
  // aquire user registrations
  $registrations = coffeeclub_user_product_registraitions_load($user);
  if(count($registrations) < 1){
    $form['no_registrations'] = array(
      '#value' => '<div>You have not yet registered a product</div>',
    );
    
    $form['add_product'] = array(
      '#type' => 'submit',
      '#value' => t('Register a product'),
    );
    
    $form['#redirect'] = 'thecoffeeclub/address/product';
    
    return $form;
  }  
  
  $rid = 0;
  if(isset($_GET['rid'])){ $rid = $_GET['rid']; }
  //if(isset($_POST['model'])){ $rid = $_POST['model']; } 
  $form = coffeeclub_user_product_register_form();
  
  
  // remove please select from the list
  $suppliers_list = $form['supplier']['#options'];
  array_shift($suppliers_list);
  
  
  // setup default form values based on first regitration in list
  $default_registration = $registrations[0]->rid;
  $default_supplier = $registrations[0]->supplier_shop;
  $product_id = $registrations[0]->serial;
  $purchase_date = date("d/m/Y",$registrations[0]->purchase_date);
  $registrations[0]->has_receipt == 0? $has_receipt_option = 1: $has_receipt_option = 0; 
  $contact_methods = $registrations[0]->contact_methods;  
  
  // load the selected registration (if any)
  $registrations_list = array();
  foreach ($registrations as $registration){
     $registrations_list[$registration->rid] = $registration->model;
     if($rid == $registration->rid ){ // default options
        $default_registration = $registration->rid;
        $default_supplier = $registration->supplier_shop;
        $product_id = $registration->serial;
        $purchase_date = date("d/m/Y",$registration->purchase_date);
        $registration->has_receipt == 0? $has_receipt_option = 1: $has_receipt_option = 0;
        $contact_methods = $registration->contact_methods;
        $special_offers = $registration->special_offers;                
     }
  }
  
  $form['special_offers'] = array(
    '#type' => 'hidden',
    '#value' => $special_offers,
  );
 
  // set form values to selected product
  $form['model']['#default_value'] = $default_registration;  
  $form['model']['#options'] = $registrations_list;
  $form['model']['#attributes'] = array('onchange' => 'form.submit(\'coffeeclub_user_registered_products_list_form\');return false;');  
  
  $form['supplier']['#default_value'] = $default_supplier; 
  $form['supplier']['#options'] = $suppliers_list;      

   
  $form['product_serial']['#value'] = $product_id;
  $form['product_serial']['#type'] = 'hidden';  
  $form['purchase_date']['#value'] = $purchase_date;

  
  $form['receipt_options_address']['#default_value'] = $has_receipt_option;
  
  if(stripos($contact_methods,'email')!==false){ $form['contact_email']['#default_value'] = 'email'; }
  if(stripos($contact_methods,'post')!==false){ $form['contact_post']['#default_value'] = 'post'; }    
  if(stripos($contact_methods,'sms')!==false){ $form['contact_sms']['#default_value'] = 'sms'; }  
  if(stripos($contact_methods,'phone')!==false){ $form['contact_phone']['#default_value'] = 'phone'; }   
  
  // this submit already exists in the product_form - it needs to be removed
  unset($form['product_submit']);
  
  // this is a hidden submit on the form which drupal passes as the submit button used
  // when select is changed. This must be the first select button on the form. 
  $form['jscript_submit'] = array(
    '#type' => 'submit',
    '#value' => 'select',
    '#prefix' => '<div style="display:none;">',
    '#suffix' => '</div>',
    //'#value' => t(''),
  );
    
  $form['additional_address'] = array(
    '#type' => 'submit',
    '#value' => t('Register another product'),
  );
    
  $form['product_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );  
  
  /*$form['update_registered_product'] = array(
    '#type' => 'submit',
    '#value' => t('Update'),
    '#weight' => 1,
  );*/
  
  
  return $form;

}

function coffeeclub_user_registered_products_list_form_submit($form, &$form_state){
  $op = $form_state['values']['op'];
  switch($op){
    case 'select':
      drupal_goto('thecoffeeclub/account/','rid='.$form_state['values']['model']); 
    return;
    case 'Register another product':
      $form_state['redirect'] = 'thecoffeeclub/address/product';     
    return;
    case 'Update':
      coffeeclub_update_product_registration($form_state['clicked_button']['#post']); // in coffeeclub.data.inc
      drupal_goto('thecoffeeclub/account/','rid='.$form_state['values']['model']);
    
    return;
    case 'Submit':
      coffeeclub_update_product_registration($form_state['clicked_button']['#post']); // in coffeeclub.data.inc
      drupal_goto('thecoffeeclub/account/','rid='.$form_state['values']['model']);      
    return;            
  }

}

/**
 * AHAH method - not in use
 *
 */  
function coffeeclub_update_register_form_js(){

  // The form is generated in an include file which we need to include manually.
  //include_once drupal_get_path('module','coffeeclub.user.inc'); 
  
    // We're starting in step #3, preparing for #4.
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  $form_build_id = $_POST['form_build_id'];
  // Step #4.
  $form = form_get_cache($form_build_id, $form_state);

  // Preparing for #5.
  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;
  $form['#programmed'] = $form['#redirect'] = FALSE;

  // Step #5.
  drupal_process_form($form_id, $form, $form_state);
  // Step #6 and #7 and #8.
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

  // Step #9.
  $choice_form = $form['choice_wrapper']['choice'];
  unset($choice_form['#prefix'], $choice_form['#suffix']);
  $output = theme('status_messages') . drupal_render($choice_form);

  // Final rendering callback.
  drupal_json(array('status' => TRUE, 'data' => $output));
}



/**
 * gets the extant address form and adapts it for additional address profile
 *
 *
 */
function coffeeclub_user_alternate_address_form(){
  global $user;
  
  profile_load_profile($user);

  $form = coffeeclub_user_address_register_form();
  
  // remove non-relevant fields
  unset($form['first_name']);
  unset($form['last_name']);
  unset($form['email_news']);
  
  isset($user->profile_a_houseno)? $house_no = $user->profile_a_houseno: $house_no = "";
  isset($user->profile_a_street1)? $street1 = $user->profile_a_street1: $street1 = "";  
  isset($user->profile_a_street2)? $street2 = $user->profile_a_street2: $street2 = "";
  isset($user->profile_a_county)? $county = $user->profile_a_county: $county = "";  
  isset($user->profile_a_postcode)? $postcode = $user->profile_a_postcode: $postcode = "";  
  isset($user->profile_a_phone)? $phone = $user->profile_a_phone: $phone = "";
  isset($user->profile_a_mobile)? $mobile = $user->profile_a_mobile: $mobile = "";
  isset($user->profile_use_alternate)? $use = $user->profile_use_alternate: $use = "";     
  
  $form['house_no']['#default_value'] = $house_no;
  $form['street1']['#default_value'] = $street1;  
  $form['street2']['#default_value'] = $street2;
  $form['county']['#default_value'] = $county;  
  $form['postcode']['#default_value'] = $postcode;  
  $form['phone']['#default_value'] = $phone;  
  $form['mobile']['#default_value'] = $mobile;
  
  $form['alternate'] = array(
    '#type' => 'checkbox',
    '#title' => 'Use this address',
    '#default_value' => $use,
  );   
  
  $form['submit_address']['#value'] = 'Update';
  $form['submit_address']['#weight'] = 1;
  
  

  
  if(isset($_GET['destination'] )){
    $form['return_destination'] = array(
      '#type' => 'submit',
      '#value' => 'Back',
    );    
  }  
    
  return $form; 
 
 
}   

  
function coffeeclub_user_alternate_address_form_submit($form, &$form_state){

  global $user;
  $edit = array(
    'profile_a_houseno' => $form_state['values']['house_no'],
    'profile_a_street1' => $form_state['values']['street1'],
    'profile_a_street2' => $form_state['values']['street2'],
    'profile_a_county' => $form_state['values']['county'],
    'profile_a_postcode' => $form_state['values']['postcode'],
    'profile_a_number_daytime' => $form_state['values']['phone'],      
    'profile_a_number_mobile' => $form_state['values']['mobile'],
    'profile_use_alternate' => $form_state['values']['alternate'],    
  
  );
  profile_save_profile( $edit, $user, 'Alternate Address' );
  
  drupal_set_message('Your alternate address details have been saved');

}  


function coffeeclub_form_user_register_alter(&$form, &$form_state) {
  // ensure there is no submit
  unset($form['#submit']);

  // replace with clone form_submit
  $form['#submit'][] = 'coffeeclub_user_register_submit';
}


/**
 *  cloned directy from user_register_submit in user.module
 *  to overide the automatic sending of welcome emails 
 * 
 * Submit handler for the user registration form.
 *
 * This function is shared by the installation form and the normal registration form,
 * which is why it can't be in the user.pages.inc file.
 */
function coffeeclub_user_register_submit($form, &$form_state) {
  global $base_url;
  $admin = user_access('administer users');

  $mail = $form_state['values']['mail'];
  $name = $form_state['values']['name'];
  if (!variable_get('user_email_verification', TRUE) || $admin) {
    // issue with this passing the password, only passed first letter of the password
    //$pass = $form_state['values']['pass'];
    $pass = $form_state['clicked_button']['#post']['pass']['pass1'];
  }
  else {
    $pass = user_password();
  };
  $notify = isset($form_state['values']['notify']) ? $form_state['values']['notify'] : NULL;
  $from = variable_get('site_mail', ini_get('sendmail_from'));
  if (isset($form_state['values']['roles'])) {
    // Remove unset roles.
    $roles = array_filter($form_state['values']['roles']);
  }
  else {
    $roles = array();
  }

  if (!$admin && array_intersect(array_keys($form_state['values']), array('uid', 'roles', 'init', 'session', 'status'))) {
    watchdog('security', 'Detected malicious attempt to alter protected user fields.', array(), WATCHDOG_WARNING);
    $form_state['redirect'] = 'user/register';
    return;
  }
  // The unset below is needed to prevent these form values from being saved as
  // user data.
  unset($form_state['values']['form_token'], $form_state['values']['submit'], $form_state['values']['op'], $form_state['values']['notify'], $form_state['values']['form_id'], $form_state['values']['affiliates'], $form_state['values']['destination']);

  $merge_data = array('pass' => $pass, 'init' => $mail, 'roles' => $roles);
  if (!$admin) {
    // Set the user's status because it was not displayed in the form.
    $merge_data['status'] = variable_get('user_register', 1) == 1;
  }
  $account = user_save('', array_merge($form_state['values'], $merge_data));
  // Terminate if an error occured during user_save().
  if (!$account) {
    drupal_set_message(t("Error saving user account."), 'error');
    $form_state['redirect'] = '';
    return;
  }
  $form_state['user'] = $account;

  watchdog('user', 'New user: %name (%email).', array('%name' => $name, '%email' => $mail), WATCHDOG_NOTICE, l(t('edit'), 'user/'. $account->uid .'/edit'));

  // The first user may login immediately, and receives a customized welcome e-mail.
  if ($account->uid == 1) {
    drupal_set_message(t('Welcome to Drupal. You are now logged in as user #1, which gives you full control over your website.'));
    if (variable_get('user_email_verification', TRUE)) {
      drupal_set_message(t('</p><p> Your password is <strong>%pass</strong>. You may change your password below.</p>', array('%pass' => $pass)));
    }

    user_authenticate(array_merge($form_state['values'], $merge_data));

    $form_state['redirect'] = 'user/1/edit';
    return;
  }
  else {
    // Add plain text password into user account to generate mail tokens.
    $account->password = $pass;
    if ($admin && !$notify) {
      drupal_set_message(t('Created a new user account for <a href="@url">%name</a>. No e-mail has been sent.', array('@url' => url("user/$account->uid"), '%name' => $account->name)));
    }
    else if (!variable_get('user_email_verification', TRUE) && $account->status && !$admin) {
      // No e-mail verification is required, create new user account, and login
      // user immediately.
      //_user_mail_notify('register_no_approval_required', $account);
      if (user_authenticate(array_merge($form_state['values'], $merge_data))) {
        drupal_set_message(t('Registration successful. You are now logged in.'));
      }
      $form_state['redirect'] = '';
      return;
    }
    else if ($account->status || $notify) {
      // Create new user account, no administrator approval required.
      $op = $notify ? 'register_admin_created' : 'register_no_approval_required';
      //_user_mail_notify($op, $account);
      if ($notify) {
        drupal_set_message(t('Password and further instructions have been e-mailed to the new user <a href="@url">%name</a>.', array('@url' => url("user/$account->uid"), '%name' => $account->name)));
      }
      else {
        drupal_set_message(t('Your password and further instructions have been sent to your e-mail address.'));
        $form_state['redirect'] = '';
        return;
      }
    }
    else {
      // Create new user account, administrator approval required.
      _user_mail_notify('register_pending_approval', $account);
      drupal_set_message(t('Thank you for applying for an account. Your account is currently pending approval by the site administrator.<br />In the meantime, a welcome message with further instructions has been sent to your e-mail address.'));
      $form_state['redirect'] = '';
      return;

    }
  }
}

/**
 *
 * @param 
 * @return
 */
function coffeeclub_todays_annual_registrations(){

//coffeeclub_annual_reminder_update_reset_test(); // testing, resets all reminer_sent to 2010
//exit();

    $today = time();

    $this_date = date('Y-m-d',$today);
    $this_date = explode("-",$this_date);
    
    // these values are passed to the database to retireve all unmarked registrations
    // for last month and this month
    $this_year = date('Y',$today);
    $this_month_date = mktime(0,0,0,($this_date[1]-1),01,$this_date[0]);    
    $next_month_date = mktime(0,0,0,($this_date[1]+1),01,$this_date[0]);
         
   //coffeeclub_load_todays_annual_product_registrations
   $todays_registraitions = coffeeclub_load_annual_product_registrations($this_year, $this_month_date, $next_month_date);  // data.inc
   
    $trc = count($todays_registraitions);
     watchdog('monitor','Found '.$trc.' candidate registraitions.', array(), WATCHDOG_NOTICE, null);
   
   foreach ($todays_registraitions as $key => $value){
      $send = false;
      $free_service = false;
      
     $reg_date = date('Y-m-d',$value->registraition_date);
     $reg_date = explode("-",$reg_date);
     $offer = $value->special_offers;

     // free service offer
     if( is_numeric(stripos($offer,"service")) == 1){ $free_service = true; }
     
    // month and date match
    if ( ($reg_date[1] <= $this_date[1]) && ($reg_date[2] <= $this_date[2]) ){ $send = true; }
    
    // now catch any missed ones from last month
    if ( ($reg_date[1] < $this_date[1]) ) { $send = true; }
    
    
    if ($send == true){
        // update the database
        coffeeclub_annual_reminder_update($value->rid, $this_year);
        $user = user_load($value->uid);

        // user uid must be set to null, or multi-regi products per user will get same product tokens
        $user->uid = null;
        $user->model = $value->model;
        $user->serial = $value->serial;
        $user->product_class = $value->name;
        
        // send email to service/customer dept to notify member has registerd a product
       if($free_service == true){
         module_invoke_all('coffeeclub', 'annual_service_reminder', $user);
       }else{
         module_invoke_all('coffeeclub', 'annual_discount_service_reminder', $user);
       }
        
    }
       
   }   
  
}
