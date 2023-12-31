function doGet(e) { 
  Logger.log( JSON.stringify(e) );
  var result = 'Ok';
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
}
else {
  var sheet_id = ''; // Spreadsheet ID
  var sheet = SpreadsheetApp.openById(sheet_id).getActiveSheet();
  var newRow = sheet.getLastRow() + 1; 
  var rowData = [];
  var Curr_Date = new Date();
  var Curr_Time = Utilities.formatDate(Curr_Date, "America/New_York", 'HH:mm:ss');

  rowData[0] = Curr_Date; // Date in column A
  rowData[1] = Curr_Time; // Time in column B

  for (var param in e.parameter) {
    Logger.log('In for loop, param=' + param);
    var value = stripQuotes(e.parameter[param]);
    Logger.log(param + ':' + e.parameter[param]);

    switch (param) {
      case 'temperature':
      rowData[2] = value; // temperature in column C
      result = 'Temperature Written on column C'; 
      break;

      case 'humidity':
      rowData[3] = value; // humidity in column D
      result += ' ,Humidity Written on column D'; 
      break; 

      case 'oxygen':
      rowData[4] = value; // oxygen levels in Column E
      result += ' ,Oxygen level Written on column E';
      break;

      case 'bpm':
      rowData[5] = value; // heartrate in Column F
      result = ' ,Bpm Written on column F ';
      break;
      
      default:
      result = "unsupported parameter";
    }
}
  Logger.log(JSON.stringify(rowData));
  var newRange = sheet.getRange(newRow, 1, 1, rowData.length);
  newRange.setValues([rowData]);
}
return ContentService.createTextOutput(result);
}
function stripQuotes( value ) {
return value.replace(/^["']|['"]$/g, "");
}
