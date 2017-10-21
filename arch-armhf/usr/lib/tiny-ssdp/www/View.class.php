<?php 

/* Class to read and display text document with php variables to be replaced
 * sample of use
	$v = new View('content.html');
	echo $v->get();	// use global variables
	echo $v->get(array('var' => 'value', 'var2' => 'other value'));	// provide replacement array
	echo $v->get(false);	// disable variable replacement
*/
class View
{
	var $file;
	var $variables;

	public function View($file)
	{
		$this->loadFile($file);
	}
	public function loadFile($file)
	{
		if (!is_readable($file))
		{
			trigger_error("File not found or not readable: '$file'", E_USER_ERROR);
		}
		else
		{
			$this->file = $file;
		}
	}
	private function variableReplace($var)
	{
		if (!isset($this->variables[$var[1]]))
		{
			trigger_error('Variable '.htmlspecialchars($var[0]).' in "'.$this->file.'" is not defined in given context.', E_USER_ERROR);
		}
		return $this->variables[$var[1]];
	}
	public function get($variables=null)
	{
		$content = file_get_contents($this->file);

		if (is_null($variables))
		{
			$this->variables = $GLOBALS;
			return preg_replace_callback('/\\$([a-zA-Z_][a-zA-Z0-9_]+|{[^}]+})/', array($this, 'variableReplace'), $content);
		}
		else if (is_array($variables))
		{
			$this->variables = $variables;
			return preg_replace_callback('/\\$([a-zA-Z_][a-zA-Z0-9_]+|{[^}]+})/', array($this, 'variableReplace'), $content);
		}

		return $content; /* replacement disabled */
	}
	public function show($variables=null)
	{
		echo $this->get($variables);
	}
}

?>
