<?php

// if(isset($_GET["sound"])) {
//    $sound = $_GET["sound"]; // get sound value from HTTP GET

   $servername = "localhost";
   $username = "Arduino";
   $password = "pass";
   $dbname = "db_arduino";

   // Create connection
   $conn = new mysqli($servername, $username, $password, $dbname);
   // Check connection
   if ($conn->connect_error) {
      die("Connection failed: " . $conn->connect_error);
   }


// Perform query
if ($result = $conn -> query("SELECT AVG(sound_value) as avgSoundVal FROM tbl_sound_readings")) {
   // echo "Returned rows are: " . mysqli_num_rows($result);
   // echo "Returned rows are: " . mysqli_fetch_all($result);

   // Numeric array
// $row = mysqli_fetch_array($result, MYSQLI_NUM);
$row = mysqli_fetch_array($result, MYSQLI_ASSOC);


// printf ("%s (%s)\n", $row[0], $row[1]);
// echo $row[0];


// $arraytest = array(, "green", "blue", "yellow");

// foreach ($row as $value) {
// echo $value;
// echo "\n";
//  }



  //create an array
//   $emparray = array();
//   while($rowTest =mysqli_fetch_assoc($result))
//   {
//       $emparray[] = $rowTest;
//   }

  echo json_encode($row);




   // Free result set
   $result -> free_result();
 }

   $conn->close();
// } else {
//    echo "sound is not set";
// }
?>
