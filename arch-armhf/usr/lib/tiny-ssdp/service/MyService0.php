<?php

$version = ((isset($_GET['version']) && is_numeric($_GET['version'])) ? intval($_GET['version'],10):1);
$action = (isset($_GET['action']) ? $_GET['action']:false);


switch($version)
{
	case 1:
	default:
		if ($action == 'control')
		{
			/* do SOAP here */
			header("HTTP/1.1 501 Action control not implemented."); exit;
			break;
		}
		else if ($action == 'eventSub')
		{
			/* do SOAP here */
			header("HTTP/1.1 501 Action eventSub not implemented."); exit;
			break;
		}
		else
		{
			header("HTTP/1.1 405 Action unknown."); exit;
		}
	break;
}



?>
