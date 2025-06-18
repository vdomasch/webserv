#!/usr/bin/php-cgi
<?php


parse_str($_SERVER['QUERY_STRING'], $params);

$n1 = isset($params['n1']) ? $params['n1'] : null;
$n2 = isset($params['n2']) ? $params['n2'] : null;
$op = isset($params['op']) ? $params['op'] : null;

$result = "";

if (!is_numeric($n1) || !is_numeric($n2))
{
	echo "<!DOCTYPE html>
<html>
	<head>
		<meta charset='UTF-8'>
		<meta name='viewport' content='width=device-width, initial-scale=1.0'>
		<link rel='icon' type='image/x-icon' href='/icons/favicon.ico'>
		<link rel='stylesheet' href='/assets/css_files/styles.css'>
		<title>calculator</title>
	</head>
	<body>
		<section class='title'>
			<a href='/index.html'>
				<h1 class='title'>Webserv</h1>
			</a>
		</section>
		<section class='block'>
			<h2>Invalid input: Both numbers must be numeric.</h2>
    		<div class='responsive-item'>
        		<a href='/calculator.html'>Back to Calculator</a>
			</div>
		</section>
	</body>
</html>";
	exit;
}

class Calculator
{
    var $a;
    var $b;


    function checkoperation($op)
    {
        switch($op)
        {
            case '+': return $this->a + $this->b;
            case '-': return $this->a - $this->b;
            case '*': return $this->a * $this->b;
            case '/': return ($this->b == 0) ? "Division by zero" : $this->a / $this->b;
            default:  return "Invalid operation";
        }
    }

    function getresult($a, $b, $op)
    {
        $this->a = $a;
        $this->b = $b;
        return $this->checkoperation($op);
    }
}

if ($n1 !== null && $n2 !== null && $op !== null) {
    $calc = new Calculator();
    $result = $calc->getresult($n1, $n2, $op);
}

echo "<!DOCTYPE html>
<html>
	<head>
		<meta charset='UTF-8'>
		<meta name='viewport' content='width=device-width, initial-scale=1.0'>
		<link rel='icon' type='image/x-icon' href='/icons/favicon.ico'>
		<link rel='stylesheet' href='/assets/css_files/styles.css'>
		<title>calculator</title>
	</head>
	<body>
		<section class='title'>
			<a href='/index.html'>
				<h1 class='title'>Webserv</h1>
			</a>
		</section>
		<section class='block'>
			<h2>Here is your result</h2>
			<div class='responsive-row'>
				<h2 style='text-align:center;'>Result: $result</h2>
			</div>
    		<div class='responsive-item'>
        		<a href='/calculator.html'>Back to Calculator</a>
			</div>
		</section>
	</body>
</html>";
?>