<?php


function coffeeclub_admin_settings_form(){

     //$vocabulary = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
     
     $vocabularies = taxonomy_get_vocabularies();
     
     /*dbg*/ //drupal_set_message('<pre>$vocabularies <br>'.print_r($vocabularies ,true).'</pre>');
     /*dbg*/ //drupal_set_message('<pre> <br>'.print_r(variable_get( 'coffeeclub_service_enquiry_types_vid', 1 ) ,true).'</pre>');
     
     $vocab_options = array();
     foreach( $vocabularies as $vocabulary ){ $vocab_options[$vocabulary->vid] = $vocabulary->name; }



  $form['coffeeclub_service_dept_email'] = array(
    '#title' => 'Service Dept. email',
    '#type' => 'textfield',
    '#description' => t('Email address for the Service Dept. Used when a Customer makes a service enquiry.'),
    '#default_value' => variable_get( 'coffeeclub_service_dept_email', '' ),    
  );
  
  $form['coffeeclub_customer_dept_email'] = array(
    '#title' => 'Customer Dept. email',
    '#type' => 'textfield',
    '#description' => t('Email address for the Customer Dept. Used when a Customer makes a general enquiry.'),
    '#default_value' => variable_get( 'coffeeclub_customer_dept_email', '' ),    
  );  
  
  $form['coffeeclub_product_class_vid'] = array(
    '#title' => 'Product classes list (ie: coffee to bean, pump esspresso)',
    '#type' => 'select',
    '#options' => $vocab_options,
    '#default_value' => variable_get( 'coffeeclub_product_class_vid', 1 ),
    '#description' => t('Please select the product classes list from the dropdown.'),
  );  
  
  $form['coffeeclub_service_enquiry_types_vid'] = array(
    '#title' => 'Service enquiry types list',
    '#type' => 'select',
    '#options' => $vocab_options,
    '#default_value' => variable_get( 'coffeeclub_service_enquiry_types_vid', 1 ),
    '#description' => t('Please select the Service enquiry types list from the dropdown.'),
  );
  
  $form['coffeeclub_customer_enquiry_types_vid'] = array(
    '#title' => 'Customer Service enquiry types list',
    '#type' => 'select',
    '#options' => $vocab_options,
    '#default_value' => variable_get( 'coffeeclub_customer_enquiry_types_vid', 1 ),
    '#description' => t('Please select the Customer enquiry types list from the dropdown.'),
  );
    
  $form['coffeeclub_service_enquiry_status_vid'] = array(
    '#title' => 'Service enquiry status list',
    '#type' => 'select',
    '#options' => $vocab_options,
    '#default_value' => variable_get( 'coffeeclub_service_enquiry_status_vid', 1 ),
    '#description' => t('Please select the Service enquiry status list from the dropdown. This vocabulary must contain either open or new as a state.'),
  );
  
  $form['coffeeclub_service_suppliers_vid'] = array(
    '#title' => 'Suppliers list',
    '#type' => 'select',
    '#options' => $vocab_options,
    '#default_value' => variable_get( 'coffeeclub_service_suppliers_vid', 1 ),
    '#description' => t('Please select the Suppliers list from the dropdown.'),
  );
  
  $form['coffeeclub_beantocup_offer_text'] = array(
    '#title' => 'Bean to cup offer',
    '#type' => 'textarea',
    '#default_value' => variable_get( 'coffeeclub_beantocup_offer_text', '' ),
    '#description' => t('Welcome offer on Bean to cup Machine registrations.'),
  );
  
  $form['coffeeclub_beantocup_free_service_text'] = array(
    '#title' => 'Bean to cup price offer',
    '#type' => 'textarea',
    '#default_value' => variable_get( 'coffeeclub_beantocup_free_service_text', '' ),
    '#description' => t('Bean to cup offer on Machines priced over &pound;799.99.'),
  );          
  
  
  
  /*$form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update')
  );*/
  
  return system_settings_form($form);

}



/**
* Validate the form.
*/
function coffeeclub_admin_settings_form_validate($form, &$form_state) {

}

/**
* form submission.
*/
function coffeeclub_admin_settings_form_submit($form, &$form_state) {
  

  
}


/**
* 
*/
function coffeeclub_admin_service_accounts_view() {
  
   if( isset($_GET['page']) ){
    $page = $_GET['page'];
   }else{
    $page = 0;
   }
   $page_length = 10;
  
  // get all users uids and names
  /*$result = db_query("
  SELECT uid, name FROM {users}
  LIMIT %d, %d
  ", (($page)*$page_length), $page_length); */
  
  $result = db_query("
  SELECT uid, name FROM {users} ORDER BY created DESC
  ");  
  
  while($row = db_fetch_object($result)) $rows[] = $row;
  
  /*dbg*/ //drupal_set_message('<pre>$rows <br>'.print_r($rows ,true).'</pre>');
  // load profile for each user
  foreach($rows as $user){
    if( $user->uid == 0 ) continue;
    profile_load_profile($user);
    $users[] = $user;
  }
  
  $pages = intval(count($users)/$page_length);
  
  // create table of users 
  $output = '<table>
   <tr>
   <th>Account number</th><th>Customer name</th><th>Products</th><th>Enquiries</th><th>Edit</th>';
  foreach( $users as $user ){
  $user_name = '';
  if( isset($user->profile_forename) && isset($user->profile_surname) ){
   $user_name = $user->profile_forename.' '.$user->profile_surname;
  }else{
   $user_name = $user->name;
  }
  
  $user_products = coffeeclub_user_product_registraitions_load($user);
  $no_of_products = count($user_products);
  $user_enquiries = coffeeclub_enquiries_load_by_user($user);
  $no_of_enquiries = count($user_enquiries);
  
   $output .= '
     <tr>
      <td>'.$user->uid.'</td>
      <td>'.$user_name.'</td>
      <td>'.$no_of_products.'</td>      
      <td>'.$no_of_enquiries.'</td>
      <td>'.l('show','admin/user/thecoffeeclub/accounts/'.$user->uid.'/account').'</td>                          
     </tr>
     '; 
  }
  $output .= 
   '</tr>
   </table>';
   
 
  //$output .= '<div>';
  
  //$pages = 10;
  /*dbg*/ //drupal_set_message('<pre> $pages<br>'.print_r($pages ,true).'</pre>');
  /*dbg*/ //drupal_set_message('<pre> $users<br>'.print_r(count($users) ,true).'</pre>');  
  
  /*for( $i=0; $i<$pages; $i++){
    $output .= '<span >'.l('['.$i.']','admin/user/thecoffeeclub/accounts',array('query' => array('page' => $i))).'</span>';  
  }
  
  
  if( ($page+1) < $pages )  $output .= '<span >'.l('next>','admin/user/thecoffeeclub/accounts',array('query' => array('page' => $page+1))).'</span>';
  if( ($page-1) > 0 )  $output .= '<span >'.l('<prev','admin/user/thecoffeeclub/accounts',array('query' => array('page' => $page+1))).'</span>';
  
  
  $output .= '</div>';*/     
  
  
  
  return $output;
  
}




/**
* 
*/
function coffeeclub_admin_service_accounts_form($form, &$form_state) {

    if(isset($form_state)){}
     // get all users uids and names
     $result = db_query("
     SELECT uid, name FROM {users}
     ");
     
     while($row = db_fetch_object($result)) $rows[] = $row;
     
     // load profile for each user
     foreach($rows as $user){
      if( $user->uid ==0 ) continue;
      profile_load_profile($user);
      $users[] = $user;
     }
     
     // create table of users 
     $output = '<table><tr><th>Account number</th><th>Customer name</th>';
     foreach( $users as $user ){
     $user_name = '';
     if( isset($user->profile_forename) && isset($user->profile_surname) ){
       $user_name = $user->profile_forename.' '.$user->profile_surname;
     }else{
       $user_name = $user->name;
     }
        
       $output .= '
       <tr>
        <td>'.$user->uid.'</td>
        <td>'.$user_name.'</td>        
       </tr>
       '; 
     }
     $output .= '</tr></table>';
     
     $form['output'] = array(
      '#type' => 'markup',
      '#value' => $output,
     );         

    $form['page_number'] = array(
    '#type' => 'hidden',
    '#value' => $page_number,
    );
    
    $form['page_next'] = array(
    '#type' => 'submit',
    '#value' => 'page next',
    );      
  
    /*$form['model'] = array(
    '#title' => t('Model'),
    '#type' => 'select',
    '#options' => array('a','b'),
    '#prefix' => '<div class="container-inline container">', 
    '#suffix' => '</div>',
    //'#required' => true,
    //'#description' => t('.'),
  );*/  
  
    return $form;
  
}

/**
* 
*/
function coffeeclub_admin_service_accounts_form_validate($form, &$form_state) {
  

  
}

/**
* 
*/
function coffeeclub_admin_service_accounts_form_submit($form, &$form_state) {
  
    $form_state['redirect'] = 'thecoffeeclub/accounts';
  
}

/**
 *  preps data for coffeeclub_admin_user_account_user
 *
 */  
function coffeeclub_admin_user_account_details($uid){
  // load user object
  $user = user_load($uid);
  
  // create user details array for account details
  $account = new stdClass();
  $address = new stdClass();
  $alternate = new stdClass();    
  isset($user->name) ? $account->name = $user->name : $account->name = '';
  isset($user->profile_forename) ? $account->first_name = $user->profile_forename : $account->first_name = '';  
  isset($user->profile_surname) ? $account->last_name = $user->profile_surname : $account->last_name = '';
  isset($user->profile_marketing_emails) ? $account->optin = $user->profile_marketing_emails : $account->optin = 0;
  
  // address
  isset($user->profile_houseno) ? $address->house = $user->profile_houseno : $address->house = '';
  isset($user->profile_street1) ? $address->address1 = $user->profile_street1 : $address->address1 = '';  
  isset($user->profile_street2) ? $address->address2 = $user->profile_street2 : $address->address2 = '';  
  isset($user->profile_county) ? $address->county = $user->profile_county : $address->county = '';
  isset($user->profile_postcode) ? $address->postcode = $user->profile_postcode : $address->postcode = '';  
  isset($user->profile_number_daytime) ? $address->phone = $user->profile_number_daytime : $address->phone = '';  
  isset($user->profile_number_mobile) ? $address->mobile = $user->profile_number_mobile : $address->mobile = '';
  
  // alternate
  isset($user->profile_a_houseno) ? $alternate->house = $user->profile_a_houseno : $alternate->house = '';
  isset($user->profile_a_street1) ? $alternate->address1 = $user->profile_a_street1 : $alternate->address1 = '';  
  isset($user->profile_a_street2) ? $alternate->address2 = $user->profile_a_street2 : $alternate->address2 = '';  
  isset($user->profile_a_county) ? $alternate->county = $user->profile_a_county : $alternate->county = '';
  isset($user->profile_a_postcode) ? $alternate->postcode = $user->profile_a_postcode : $alternate->postcode = '';  
  isset($user->profile_a_number_daytime) ? $alternate->phone = $user->profile_a_number_daytime : $alternate->phone = '';  
  isset($user->profile_a_number_mobile) ? $alternate->mobile = $user->profile_a_number_mobile : $alternate->mobile = '';  
  isset($user->profile_use_alternate) ? $alternate->use_alternate = $user->profile_use_alternate : $alternate->use_alternate = false;  
  
  $account->title = '<h3>Account: '.$user->uid.'</h3>'.
    '<div>Login: '.$account->name.'</div>'. 
    '<div>Name: '.$account->first_name.' '.$account->last_name.'</div>';      
    
  // create products details
  
  
  // create enquiry details
    
  $details = array();
  $details['uid'] = $user->uid;
  $details['account'] = $account;
  $details['address'] = $address;   
  $details['alternate'] = $alternate;  
  
  return $details;
  
}

/**
 *  prepairs the service view of the user account details
 *
 */  
function coffeeclub_admin_user_account_user($uid){

  $details = coffeeclub_admin_user_account_details($uid);
  
  // alert service which address the customer wishes used
  $use_alternate_address = '';
  if($details['alternate']->use_alternate){ $use_alternate_address = '<div>Wishes alternate address to be used</div>';}
  else{$use_alternate_address = '<div>Wishes primary address to be used</div>';}
  
  // show marketing opt in choice
  $optin = '';
  if($details['account']->optin){ $optin = '<div>Will accept marketing emails</div>';}
  else{$optin = '<div>Does not wish to recieve marketing emails<br /></div>';}  
  
  
  $address_table = '<table>
  <tr><th></th><th>Primary Address</th><th>Alternate Address</th></tr>';
  
  foreach($details['alternate'] as $key => $line){
    $address_table .= '<tr>';
    if($key != 'use_alternate'){
      $address_table .= '<td>'.$key.'</td><td>'.$details['address']->$key.'</td>';
      $address_table .= '<td>'.$line.'</td>';
    }    
  }    
    $address_table .= '</tr>';

  
  $address_table .= '</table>';
  
  
  $details['address table'] =  $use_alternate_address.$optin.$address_table;

 return $details;
} 

/**
 *
 *
 */
function coffeeclub_admin_user_products($uid){
  $details = coffeeclub_admin_user_account_details($uid);
  // coffeeclub_user_product_registraitions_load requires a user object with a uid attribute
  // so create a user object with only the uid value
  $user->uid = $uid;
  $products = coffeeclub_user_product_registraitions_load($user);
  
  // prepair for no products
  $products_table = '<div>No products</div>';
  $details['products table'] = $products_table;
  if( count($products) < 1 ){ return $details; }
  
  // opt in marketing choice
  // show marketing opt in choice
  $optin = '';
  if($details['account']->optin){ $optin = '<div>Will accept marketing emails</div>';}
  else{$optin = '<div>Does not wish to recieve marketing emails<br /></div>';}
  
  // products have been registered
  $details['account']->title .= '<div>Products registered: '.count($products).'</div>';
  $details['account']->title .= $optin;  
  //$details['account']->title .= '<div><br />'.l('edit product offer email','admin/settings/actions/configure').'</div><br />';
  $details['account']->title .= '<div><br />'.l('edit product offer email','admin/user/thecoffeeclub/settings').'</div><br />';
     
  $products_table = '<div>';
  $forms = '';
  foreach( $products as $product ){
    $forms .= drupal_get_form('coffeeclub_admin_user_product_form',$product);
  }  
  
  $products_table .= $forms;
  $products_table .= '</div><br />';  
  
  // push the table onto the details stack
  $details['products table'] = $products_table;  

  return $details;  
}

/**
 * Template for multi-form products page
 * @param $form_state, $product
 * a product to populate the form with
 * 
 * @return
 * a product services form      
 *
 */ 
function coffeeclub_admin_user_products_form(&$form_state, $product){

  $form['rid'] = array(
  '#type' => 'hidden',
  '#value' => $product->rid,
  );
  
  
  $products_table = '';
  $product->has_receipt == 1? $product->has_receipt = 'yes': $product->has_receipt = 'no'; 
  $products_table .= '<table class="product-details" >';  
  $products_table .= '<tr><th></th><th></th></tr>';
  $products_table .= '<tbody>';    
  $products_table .= '<tr class="even" ><td>class</td><td>'.$product->name.'</td></tr>';    
  $products_table .= '<tr><td>model</td><td>'.$product->model.'</td></tr>';
  $products_table .= '<tr><td>serial</td><td>'.$product->serial.'</td></tr>';
  $products_table .= '<tr><td>purchase date</td><td>'.date("d/m/Y",$product->purchase_date).'</td></tr>';
  $products_table .= '<tr><td>registration date</td><td>'.date("d/m/Y",$product->registraition_date).'</td></tr>';    
  $products_table .= '<tr><td>where purchased</td><td>'.$product->supplier_shop.'</td></tr>';
  $products_table .= '<tr><td>has proof of purchase</td><td>'.$product->has_receipt.'</td></tr>';
  $products_table .= '<tr><td>contact methods</td><td>'.str_replace(":",", ",$product->contact_methods).'</td></tr>';    
  $products_table .= '<tr><td>premium customer</td><td></td></tr>';
    
  $form['product_table_top'] = array(
  '#type' => 'markup',
  '#value' => $products_table,
  );
  
  if(stripos($product->special_offers,'descaler') !== false){
    $form['scaler_dispatched'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm descaler dispatch'),
    //'#description' => t('The member is awaiting dispatch of their free descaler. Click the button to confirm the descaler has been dispatched.'),
    '#prefix' => '<tr><td colspan="2" >',
    '#suffix' => '</td></tr><tr><td colspan="2" >'.t('The member is awaiting dispatch of their free descaler. Click the button to confirm the descaler has been dispatched.').'</td></tr>',
    );  
  }
  
  /*if(stripos($product->special_offers,'service') !== false){
    $form['service_dispatched'] = array(
    '#type' => 'submit',
    '#value' => t('Confirm service arrainged'),
    //'#description' => t('The member is awaiting notification of their free anual service date.'),
    '#prefix' => '<tr><td colspan="2" >',
    '#suffix' => '</td></tr><tr><td colspan="2" >'.t('The member is awaiting notification of their free annual service date.').'</td></tr>',
    );  
  }*/  

  $products_table = '';
  $products_table .= '</tbody>';    
  $products_table .= '</table>';    
  $form['product_table_close'] = array(
  '#type' => 'markup',
  '#value' => $products_table,
  );    
   
  return $form;
}

/**
 *
 *
 */  
function coffeeclub_admin_user_product_form_submit($form, &$form_state){
  $form_values = $form_state['clicked_button']['#post'];
  $rid = $form_values['rid'];
  $op = $form_values['op'];
  
  $product = coffeeclub_user_product_registraition_load($rid);

  $special_offers = $product->special_offers;

  switch($op){
    case 'Confirm descaler dispatch':
      $special_offers = str_replace("descaler:","",$special_offers);
    break;
    case 'Confirm service arrainged':
      //$special_offers = str_replace("service:","",$special_offers);    
    break;    
  }
  
  coffeeclub_update_product_registration_special_offers($rid, $special_offers);

}
   


/*function coffeeclub_admin_user_products1($uid){

  $details = coffeeclub_admin_user_account_details($uid);
  // coffeeclub_user_product_registraitions_load requires a user object with a uid attribute
  // so create a user object with only the uid value
  $user->uid = $uid;
  $products = coffeeclub_user_product_registraitions_load($user);
  
  // prepair for no products
  $products_table = '<div>No products</div>';
  $details['products table'] = $products_table;
  if( count($products) < 1 ){ return $details; }
  
  // opt in marketing choice
    // show marketing opt in choice
  $optin = '';
  if($details['account']->optin){ $optin = '<div>Will accept marketing emails</div>';}
  else{$optin = '<div>Does not wish to recieve marketing emails<br /></div>';}
  
  // products have been registered
  $details['account']->title .= '<div>Products registered: '.count($products).'</div>';
  $details['account']->title .= $optin;  
  //$details['account']->title .= '<div><br />'.l('edit product offer email','admin/settings/actions/configure').'</div><br />';
  $details['account']->title .= '<div><br />'.l('edit product offer email','admin/user/thecoffeeclub/settings').'</div><br />';
     
  $products_table = '<div>';
  foreach( $products as $product ){
    $product->has_receipt == 1? $product->has_receipt = 'yes': $product->has_receipt = 'no'; 
    $products_table .= '<table class="product-details" >';  
    $products_table .= '<tr><th></th><th></th></tr>';
    $products_table .= '<tbody>';    
    $products_table .= '<tr class="even" ><td>class</td><td>'.$product->name.'</td></tr>';    
    $products_table .= '<tr><td>model</td><td>'.$product->model.'</td></tr>';
    $products_table .= '<tr><td>serial</td><td>'.$product->serial.'</td></tr>';
    $products_table .= '<tr><td>purchase date</td><td>'.date("d/m/Y",$product->purchase_date).'</td></tr>';
    $products_table .= '<tr><td>registration date</td><td>'.date("d/m/Y",$product->registraition_date).'</td></tr>';    
    $products_table .= '<tr><td>where purchased</td><td>'.$product->supplier_shop.'</td></tr>';
    $products_table .= '<tr><td>has proof of purchase</td><td>'.$product->has_receipt.'</td></tr>';    
    $products_table .= '<tr><td>premium customer</td><td></td></tr>';
    $products_table .= '<tr><td>contact methods</td><td>'.str_replace(":",", ",$product->contact_methods).'</td></tr>';
    //$products_table .= '<tr><td>DISPATCH</td><td>AWAITING ?</td></tr>';    
    //$products_table .= '<tr><td>'.l('send product offer email','').'</td><td></td></tr>';        
    $products_table .= '</tbody>';    
    $products_table .= '</table>';
    
    if($product->special_offers != '' ){
          $products_table .= '<div>'.$product->special_offers.'</div>';
    }    
      
  }
  
  $products_table .= '</div><br />';  
  
  // push the table onto the details stack
  $details['products table'] = $products_table;  

 return $details;
}
*/

/**
 *
 *
 */  
function coffeeclub_admin_user_enquiries($uid){

  $details = coffeeclub_admin_user_account_details($uid);
  
  $user->uid = $uid;
  $enquiries = coffeeclub_enquiries_load_by_user($user);
  
  $pages = 0; // page counter

  // prepaire for no enquiries
  $enquiries_table = '<div>No enquiries</div>';
  $details['enquiries table'] = $enquiries_table;
  if( count($enquiries) < 1 ){ return $details; }  
  
  // enquiries have been made
  $details['account']->title .= '<div>Enquiries registered: '.count($enquiries).'</div>';

  foreach( $enquiries as $enquiry ){
    $forms .= drupal_get_form('coffeeclub_admin_user_enquiry_form', $enquiry);      
  }
  
  $details['enquiries table'] = $forms;  

 return $details;
}

/**
 * Implements hook_forms()
 * hook must be module name
 *
 */   
function coffeeclub_forms($form_id){

  $forms['coffeeclub_admin_user_enquiry_form'] = array(
  'callback' => 'coffeeclub_admin_user_enquiries_form');
  
  $forms['coffeeclub_admin_user_product_form'] = array(
  'callback' => 'coffeeclub_admin_user_products_form');  
  
  return $forms;
}




/**
 * Template for multi-form enqiries page
 * @param $form_state, $enquiry
 * an enquiry to populate the form with
 * 
 * @return
 * an enquiry services form      
 *
 */ 
function coffeeclub_admin_user_enquiries_form(&$form_state, $enquiry ){

  $vid = variable_get('coffeeclub_service_enquiry_status_vid',1);
  
  $enquiry_status_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  $enquiry_status_list = array();
  foreach( $enquiry_status_taxonomy as $item) { $enquiry_status_list[$item->tid] = $item->name; }
  $default_enquiry_status = $enquiry->status_tid;
  
  $vid = variable_get('coffeeclub_service_enquiry_types_vid',1);
  $enquiry_type_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  $enquiry_type_list = array();
  foreach( $enquiry_type_taxonomy as $item) { $enquiry_type_list[$item->tid] = $item->name; }
  
    
  $form['eid'] = array(
  '#type' => 'hidden',
  '#value' => $enquiry->eid,
  );
  
  $form['uid'] = array(
  '#type' => 'hidden',
  '#value' => $enquiry->uid,
  );  
    
  $form['header'] = array(
  '#type' => 'markup',
  '#value' => '<table class="enquiry-details" >',
  );  

  $form['enquiry_text'] = array(
  '#type' => 'markup',
  '#value' => $enquiry->eid,
  '#prefix' => '<tr class="odd" ><td>enquiry no</td><td>', 
  '#suffix' => '</td></tr>',
  );
  
  $form['enquiry_type_text'] = array(
  '#type' => 'markup',
  '#value' => $enquiry_type_list[$enquiry->tid],
  '#prefix' => '<tr class="" ><td>enquiry type</td><td>', 
  '#suffix' => '</td></tr>',
  );  
  
  $form['model'] = array(
  '#type' => 'markup',
  '#value' => $enquiry->model.'. serial: '.$enquiry->serial,
  '#prefix' => '<tr><td>machine</td><td>', 
  '#suffix' => '</td></tr>',
  );  
  
  $form['customer_message'] = array(
  '#type' => 'markup',
  '#value' => $enquiry->customer_message,
  '#prefix' => '<tr class="odd" ><td>customer message</td><td>', 
  '#suffix' => '</td></tr>',
  //'#weight' => -7,  
  );  
  
  $form['service_message'] = array(
  '#type' => 'textarea',
  '#value' => $enquiry->service_message,
  '#prefix' => '<tr><td>service message</td><td>', 
  '#suffix' => '</td></tr>',
  );
  
  $form['submit_reply'] = array(
  '#type' => 'submit',
  '#value' => 'update message',
  '#prefix' => '<tr><td></td><td>', 
  '#suffix' => '</td></tr>',
  //'#weight' => 9,
  );
  
      
  $form['submit_status'] = array(
  '#type' => 'submit',
  '#value' => 'update status',

  '#prefix' => '<tr><td>', 
  '#suffix' => '</td>',  
  //'#weight' => 9,
  );
  
  $form['status_tid'] = array(
  '#type' => 'select',
  '#options' => $enquiry_status_list,
  '#default_value' => $default_enquiry_status,
  '#prefix' => '<td>', 
  '#suffix' => '</td></tr>',  

  );
  
    
  $form['footer'] = array(
  '#type' => 'markup',
  '#value' => '</table>',
  //'#weight' => 10,   
  );
      
  return $form;
  
}

function coffeeclub_admin_user_enquiry_form_submit( $form, &$form_state ){

  $form_values = $form_state['clicked_button']['#post'];
  $eid =  $form_values['eid']; // enquiry id
  $uid =  $form_values['uid'];  
  
  // save message
  $result = coffeeclub_enquiry_service_message_update($eid, $form_values['service_message']); // in coffeeclub.data.inc
  
  // save status
  $result = coffeeclub_enquiry_service_status_update($eid, $form_values['status_tid'] );  //in coffeeclub.data.inc
  
  // send emails
  // load user
  $user = user_load($uid);
  
  // retrieve current enquiry status
  $vid = variable_get('coffeeclub_service_enquiry_status_vid',1);
  $service_enquiry_taxonomy = taxonomy_get_tree($vid, $parent = 0, $depth = -1, $max_depth = NULL);
  $service_enquiry_list = array();
  foreach ($service_enquiry_taxonomy as $item) { if ($item->tid == $form_values['status_tid']){ $status_text = $item->name; break; } }


/*     //Fire Block Viewed Trigger
8.    module_invoke_all('onthisdate', 'block_viewed', $user);
*/
 
  // add additional data to the user object to be passed to email 
  $user->service_message = htmlentities($form_values['service_message']);
  $user->enquiry_status = $status_text; 
  module_invoke_all('coffeeclub', 'service_enquiry_update', $user);
  
  drupal_set_message('Customer record updated. Email sent to Customer.');  
}





