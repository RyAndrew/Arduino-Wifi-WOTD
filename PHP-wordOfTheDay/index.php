<?php

// http://dictionary.reference.com/

// http://dictionary.reference.com/wordoftheday/

// RSS
// http://dictionary.reference.com/wordoftheday/wotd.rss
/*
SAMPLE RSS XML RESPONSE:

<?xml version="1.0"?>
<rss version="2.0">
<channel>
<title>Dictionary.com Word of the Day</title>
<link>http://www.dictionary.com/wordoftheday/</link>
<description>A new word is presented every day with its definition and example sentences from actual published works.</description>
<language>en-us</language>
<copyright>Copyright 2014 Dictionary.com, LLC</copyright>
<docs>http://blogs.law.harvard.edu/tech/rss</docs>
<lastBuildDate>Mon, 27 Oct 2014 00:00:00 -0700</lastBuildDate>
<ttl>720</ttl>
<image>
<title>Dictionary.com Word of the Day</title>
<url>http://sp.dictionary.com/dictstatic/g/d/dictionary_logo_sm.gif</url>
<link>http://www.dictionary.com/wordoftheday/</link>
<width>126</width>
<height>29</height>
<description>Free online dictionary, thesaurus and reference guide, crossword puzzles and other word games, online translator and Word of the Day.</description>
</image>
<item>
<title>odontoid: Dictionary.com Word of the Day</title>
<link>http://www.dictionary.com/wordoftheday/archive/2014/10/27.html?src=rss</link>
<pubDate>Mon, 27 Oct 2014 00:00:00 -0700</pubDate>
<guid isPermaLink="true">http://www.dictionary.com/wordoftheday/archive/2014/10/27.html?src=rss</guid>
<description>odontoid: of or resembling a tooth; toothlike.</description>
</item>
</channel>
</rss>
*/

new wordOfTheDay; // instantiate the class below

class wordOfTheDay {

	function __construct(){
	
		$this->wordOfTheDayRssUrl = 'http://dictionary.reference.com/wordoftheday/wotd.rss';
		$this->curlUseragent = 'Arduino-PHP-API';
		$this->cacheTimePath = './api_cache_timestamp.txt';
		$this->cacheDataPath = './api_cache.json';
		$this->currentTime = time();
		$this->cacheTime = $this->currentTime;
		$this->apiMinCallIntervalSeconds = 1 * 60 * 60; // 1 hour
		
		$this->getWordOfTheDay(); //do the deed
	}
	
	function getWordOfTheDay(){
	
		if(FALSE === $wordOfTheDay = $this->checkApiCache()){ //check the cache to avoid slamming the API
			$wordOfTheDay = $this->getWordOfTheDayRss();
		}
		
		echo '|'.$wordOfTheDay['word'] .'|'. $wordOfTheDay['definition'] .'|';
	
	}

	function checkApiCache(){
	
		if(file_exists($this->cacheTimePath)){
			if( FALSE === $this->cacheTime = file_get_contents($this->cacheTimePath)){
				die("Failed to read cache refresh timing file! File: {$this->cacheTimePath}");
			}
		}else{
			return FALSE;
		}
		$cacheDelta = $this->currentTime - $this->cacheTime;
		
		if ($cacheDelta >= $this->apiMinCallIntervalSeconds){
			return FALSE;
		}else{
			if(FALSE === $cacheData = file_get_contents($this->cacheDataPath)){
				return FALSE;
			}
			return json_decode($cacheData, TRUE); //decode JSON into array
		}
	}

	function getWordOfTheDayRss(){
	
		$curlInst = curl_init($this->wordOfTheDayRssUrl);
		curl_setopt_array($curlInst, array(
			CURLOPT_USERAGENT => $this->curlUseragent,
			CURLOPT_RETURNTRANSFER => TRUE
		));
		
		if(FALSE === $curlReturnXml = curl_exec($curlInst) ){
			echo "Curl Error!<BR>";
			if($errno = curl_errno($curlInst)) {
				$error_message = curl_strerror($errno);
				echo "Error # ({$errno}):\n {$error_message}<BR>";
			}
			exit;
		}
		curl_close($curlInst);
		
		$doc = new DOMDocument();
		
		if(FALSE === $doc->loadXML($curlReturnXml) ){
			echo "Error parsing XML!\r\n";
			exit;
		}
		
		$tagItem = $doc->getElementsByTagName('item');
		if($tagItem->length < 1 ){
			echo "Bad format from RSS XML API! Missing Item Tag\r\n";
			exit;
		}
		$tagItem = $tagItem->item(0);
		
		$tagDescription = $tagItem->getElementsByTagName('description');
		if($tagDescription->length < 1 ){
			echo "Bad format from RSS XML API! Missing Description Tag Within Item Tag\r\n";
			exit;
		}
		$wordOfTheDay = $tagDescription->item(0)->textContent;
		
		$wordOfTheDay = explode(': ', $wordOfTheDay);
		
		$wordOfTheDay = array(
			'word'=>ucfirst(strtolower(trim($wordOfTheDay[0]))), //do some formatting
			'definition'=>strtoupper(substr(trim($wordOfTheDay[1]),0,1)) . substr(trim($wordOfTheDay[1]),1)
		);
		
		//debug
		//print_r($wordOfTheDay);
		
		//write cache data
		file_put_contents($this->cacheDataPath, json_encode($wordOfTheDay));
		
		//write time files cached
		file_put_contents($this->cacheTimePath, time());
		
		return $wordOfTheDay;
	}
}

?>