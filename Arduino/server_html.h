
#include <Time.h>
#include <TimeAlarms.h>

String webSite;
String T1_time,T2_time,T3_time,T4_time;
bool T1_e,T2_e,T3_e,T4_e;

/*
 * basing website upon basic idea found at:
 * https://medium.com/@beyondborders/beginner-css-grid-sticky-navigation-scrolling-content-7c4de0a8d1dc
 * https://css-tricks.com/fun-viewport-units/
 * 
 * Will have the following pages:
 * Main page - 
 *    should show current data, possibly graphs
 *      Temps, Humidity, etc
 *      Water level readings
 *      Pump status?
*     Current time
 *    Should have manual feed button for auto-feeder setups
 * Log Page - 
 *    Show log of past notes
 *    Give place to add new note
 *    Create simple buttons for quick logs of common things?
 *      Feed fish
 *      Environment/water quality measurements
 *    Download logs (csv file?)
 * Set time page -
 *    View current time and update it.
 *      
 *    
 * 
 */


void buildWebsite()
{

  webSite = "<!DOCTYPE HTML>\n";
  webSite += "<META name='viewport' content='width=device-width, initial-scale=1'>\n";
  webSite += "<head>\n";
  webSite += "<script src=\"plotly-basic.js\"></script>";
  webSite += "<link rel=\"stylesheet\"\n";
  webSite += "href=\"MainStyle.css\">\n";

  webSite += "<script src=\"jquery-3.6.0.slim.min.js\"></script>\n";
  webSite += "<script type=\"text/javascript\" src=\"jquery.timepicker.js\"></script>\n";
  webSite += "<link rel=\"stylesheet\" type=\"text/css\" href=\"jquery.timepicker.css\">\n";

  
  webSite += "</head>\n";
  webSite += "<BODY onload=\"init()\">\n";
  webSite += "<div class=\"container\">\n";
  webSite += "<div class=\"header\"><b>ENDEAVOR AQUAPONICS MODULE</b></div>\n";
  
  webSite += "<div class=\"body\">\n";
  webSite += "put graphs here...\n";
  webSite += "</div>\n";

  webSite += "<div class=\"buttons\">\n";
  webSite += "<button onclick=\"feed()\">FEED NOW</button><br>   \n";
  webSite += "Current Time: ";
  char timestr[30]="";
  if(timeStatus()==timeSet)sprintf(timestr,"%02i/%02i/%i_%02i:%02i:%02i",month(),day(),year(),hour(),minute(),second());
  webSite += String(timestr) + " <button onclick=\"updatetime()\">Set Clock Time</button><br><br>\n";
  
  webSite += "<b>FEED TIME1:  </b><input id=\"alarm1\" type=\"text\" class=\"time\" value=\"" + T1_time + "\">\n";
  webSite += "  <input type=\"checkbox\" id=\"alarm1cbx\" name=\"alarm1cbx\"";
  if(T1_e) webSite +=" checked";
  webSite += "><label for=\"alarm1cbx\">ENABLE</label><br>\n";
  
  webSite += "<b>FEED TIME2:  </b><input id=\"alarm2\" type=\"text\" class=\"time\" value=\"" + T2_time + "\">\n";
  webSite += "  <input type=\"checkbox\" id=\"alarm2cbx\" name=\"alarm2cbx\"";
  if(T2_e) webSite +=" checked";
  webSite += "><label for=\"alarm2cbx\">ENABLE</label>\n";
  
  webSite += "<b>FEED TIME3:  </b><input id=\"alarm3\" type=\"text\" class=\"time\" value=\"" + T3_time + "\">\n";
  webSite += "  <input type=\"checkbox\" id=\"alarm3cbx\" name=\"alarm3cbx\"";
  if(T3_e) webSite +=" checked";
  webSite += "><label for=\"alarm3cbx\">ENABLE</label>\n";
  
  webSite += "<b>FEED TIME4:  </b><input id=\"alarm4\" type=\"text\" class=\"time\" value=\"" + T4_time + "\">\n";
  webSite += "  <input type=\"checkbox\" id=\"alarm4cbx\" name=\"alarm4cbx\"";
  if(T4_e) webSite +=" checked";
  webSite += "><label for=\"alarm4cbx\">ENABLE</label>\n";
  
  webSite += "<br><br><button onclick=\"savealarms()\">Save Alarm Settings</button><br><br>\n";
  
  webSite += "</div>\n";
  webSite += "<script>\n";
  webSite += "$(function() {\n";
  webSite += "  $('#alarm1').timepicker({ 'step': 30 });\n";
  webSite += "  $('#alarm2').timepicker({ 'step': 30 });\n";
  webSite += "  $('#alarm3').timepicker({ 'step': 30 });\n";
  webSite += "  $('#alarm4').timepicker({ 'step': 30 });\n";
  webSite += "});\n";
  webSite += "</script>\n";
  
  webSite += "<div class=\"footer\">\n";
  webSite += "Created by Endeavor Labs\n";
  webSite += "</div>\n";
  
  webSite += "</div>\n";
  
  webSite += "<script>\n";
  webSite += "xmlHttp=createXmlHttpObject();\n";

  webSite += "function createXmlHttpObject(){\n";
  webSite += "  if(window.XMLHttpRequest){\n";
  webSite += "    xmlHttp=new XMLHttpRequest();\n";
  webSite += "  }else{\n";
  webSite += "    xmlHttp=new ActiveXObject('Microsoft.XMLHTTP');\n";
  webSite += "  }\n";
  webSite += "  return xmlHttp;\n";
  webSite += "}\n";
  
  webSite += "function updatetime()\n";
  webSite += "{\n";
  webSite += "  var date = new Date();\n";
  webSite += "  var month = date.getMonth()+1;\n";
  webSite += "  var day = date.getDate();\n";
  webSite += "  var year = date.getFullYear();\n";
  webSite += "  var hour = date.getHours();\n";
  webSite += "  var minute = date.getMinutes();\n";
  webSite += "  var sec = date.getSeconds();\n";
  webSite += "  if(xmlHttp.readyState==0||xmlHttp.readyState==4){\n";
  webSite += "    xmlHttp.open('PUT','settime?MM=' + month + '&D=' +  day + '&Y=' + year + '&H=' + hour + '&m=' + minute + '&s=' + sec,true);\n";
  webSite += "    xmlHttp.send(null);\n";
  webSite += "  }\n";
  //refresh webpage
  webSite += "  setTimeout(() => {\n";
  webSite += "  location.reload();\n";
  webSite += "  },200);\n";
  webSite += "}\n";

  webSite += "function init()\n";
  webSite += "{\n";
  //update this to show the current time
  //webSite += "  var sliderValue = document.getElementById(\"whiteval1\").value;\n";
  //webSite += "  document.getElementById(\"textwhiteval1\").innerHTML = sliderValue;\n";
  webSite += "}\n";
  
  webSite += "function feed()\n";
  webSite += "{\n";
  webSite += "  if(xmlHttp.readyState==0||xmlHttp.readyState==4){\n";
  webSite += "    xmlHttp.open('PUT','feed',true);\n";
  webSite += "    xmlHttp.send(null);\n";
  webSite += "  }\n";
  webSite += "}\n";

  //--------------------------------------
  
  webSite += "function savealarms()\n";
  webSite += "{\n";
  webSite += "  var outstr='';\n";
  
  webSite += "  if(document.getElementById(\"alarm1cbx\").checked){\n";
  webSite += "    if(document.getElementById(\"alarm1\").value != \"\"){\n";
  webSite += "     outstr = outstr + 'savealarms?E1=' + document.getElementById(\"alarm1\").value;\n";
  webSite += "    }\n";
  webSite += "  }\n";
  
  webSite += "  if(document.getElementById(\"alarm2cbx\").checked){\n";
  webSite += "    if(document.getElementById(\"alarm2\").value != \"\"){\n";
  webSite += "      if(outstr != \"\"){outstr = outstr + '&E2=';}else{outstr='savealarms?E2=';}\n";
  webSite += "      outstr = outstr + document.getElementById(\"alarm2\").value;\n";
  webSite += "    }\n";
  webSite += "  }\n";
  
  webSite += "  if(document.getElementById(\"alarm3cbx\").checked){\n";
  webSite += "    if(document.getElementById(\"alarm3\").value != \"\"){\n";
  webSite += "      if(outstr != \"\"){outstr = outstr + '&E3=';}else{outstr='savealarms?E3=';}\n";
  webSite += "      outstr = outstr + document.getElementById(\"alarm3\").value;\n";
  webSite += "    }\n";
  webSite += "  }\n";
  
  webSite += "  if(document.getElementById(\"alarm4cbx\").checked){\n";
  webSite += "    if(document.getElementById(\"alarm4\").value != \"\"){\n";
  webSite += "      if(outstr != \"\"){outstr = outstr + '&E4=';}else{outstr='savealarms?E4=';}\n";
  webSite += "      outstr = outstr + document.getElementById(\"alarm4\").value;\n";
  webSite += "    }\n";
  webSite += "  }\n";

  webSite += "  if(xmlHttp.readyState==0||xmlHttp.readyState==4){\n";
  webSite += "    xmlHttp.open('PUT',outstr,true);\n";
  webSite += "    xmlHttp.send(null);\n";
  webSite += "  }\n";
  webSite += "}\n";

  //---------------------------------------------
  webSite += "function restoreall()\n";
  webSite += "{\n";
  //webSite += "  if(xmlHttp.readyState==0||xmlHttp.readyState==4){\n";
  //webSite += "    xmlHttp.open('PUT','RESTORE',true);\n";
  //webSite += "    xmlHttp.send(null);\n";
  //webSite += "  }\n";
  webSite += "  setTimeout(() => {\n";
  webSite += "  location.reload();\n";
  webSite += "  },200);\n";
  webSite += "}\n";
  
  webSite += "</script>\n";
  webSite += "</BODY>\n";
  webSite += "</HTML>\n";
  
}
