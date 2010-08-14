package jp.dds.vsdroid;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class Config extends Activity {
	
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.config);
		
		//Button button = (Button)findViewById(R.id.Button01);
		
		Bundle extras = getIntent().getExtras();
		if( extras != null ){
			TextView textView = ( TextView )findViewById( R.id.TextView_Message );
			textView.setText( extras.getCharSequence( "Message" ));
			//EditText editText = (EditText)findViewById(R.id.EditText01);
			//editText.setText(extras.getCharSequence("TEXT"));
		}
		
		/*
		button.setOnClickListener(new View.OnClickListener() {
			
			@Override
			public void onClick(View v) {
				Intent intent = new Intent();
				EditText editText = (EditText)findViewById(R.id.EditText01);
				CharSequence text = editText.getText();
				intent.putExtra("TEXT", text);
				setResult(RESULT_OK, intent);
				finish();
			}
			 
		});
		*/
	}
	
}
