//rotations:: replace with the cheaper ones





#include "ofApp.h"


#include <iostream>

float fix=1;

//0 is picaputre, 1 is usbinput
bool inputswitch=1;

//0 is sd aspect ratio
//use definitely with all of the VSERPI devices 
//and anything else doing like 480i/p over hdmi
//1 is corner cropping to fill the screen
int hdmi_aspect_ratio_switch=0;

ofFbo aspect_fix_fbo;



int width=320;
int height=240;

bool osc_draw_switch=1;

//oscillator stuffs
float x_rate_adder=0.0;
float x_axis=0.0;
float x_frequency=0.0;
float x_argument=0.0;
float x_amp=1.0;


float y_rate_adder=0.0;
float y_axis=0.0;
float y_frequency=0.0;
float y_argument=0.0;
float y_amp=1.0;




unsigned char* video_data;
unsigned char* displacement_data;

const int fbob=480;

int framedelayoffset=0;
unsigned int framecount=0;

ofImage pastFrames_img[fbob];

//ofPixels pastFrames_pix[fbob];

//oscillator stuffs
float osc0_rate=0.0;
//float osc0_axis=0.0;
float osc0_frequency=0.0;
float osc0_argument=0.0;
float osc0_amp=0.0;
float osc0_rate_adder=0;

ofVec2f osc0_axis;
float osc0_angle=0;
int osc0_shape=0;
//radial is a radiating switch
//0 is linear, 1 is circular, 2 is parabolic, 3 is cubic, 4 is spiraling
int osc0_radial=0;

float osc1_rate=0.0;
//float osc1_axis=0.0;
float osc1_frequency=0.0;
float osc1_argument=0.0;
float osc1_amp=0.0;
float osc1_rate_adder=0;

ofVec2f osc1_axis;
float osc1_angle=0;
int osc1_shape=0;
int osc1_radial=0;

int overflow_switch=0;

bool osc_freeze=0;


float red_offset=0;
float green_offset=0;
float blue_offset=0;

int freeze_upper=fbob;
int freeze_lower=0;

ofTexture time_displaced_texture;
ofTexture oscillator_texture2;

ofPixels picapture_pixels;

//delete these

float c1=0.0;
float c2=0.0;
float c3=0.0;
float c4=0.0;
float c5=0.0;
float c6=0.0;
float c7=0.0;
float c8=0.0;
float c9=0.0;
float c10=0.0;
float c11=0.0;
float c12=0.0;
float c13=0.0;
float c14=0.0;
float c15=.0;
float c16=.0;



//p_lock biz
//maximum total size of the plock array
const int p_lock_size=240;


//p_lock_switch turns on and off p_locks
bool p_lock_switch=0;

bool p_lock_erase=0;

//maximum number of p_locks available...maybe there can be one for every knob
//for whatever wacky reason the last member of this array of arrays has a glitch
//so i guess just make an extra array and forget about it for now
const int p_lock_number=17;

//so how we will organize the p_locks is in multidimensional arrays
//to access the data at timestep x for p_lock 2 (remember counting from 0) we use p_lock[2][x]
float p_lock[p_lock_number][p_lock_size];


//smoothing parameters(i think for all of these we can array both the arrays and the floats
//for now let us just try 1 smoothing parameter for everything.
float p_lock_smooth=.5;


//and then lets try an array of floats for storing the smoothed values
float p_lock_smoothed[p_lock_number];


//turn on and off writing to the array
bool p_lock_0_switch=1;

//global counter for all the locks
int p_lock_increment=0;

void incIndex()  // call this every frame to calc the offset eeettt
{
    
    framecount++;
    framedelayoffset=framecount % fbob;
}



//--------------------------------------------------------------
void ofApp::setup() {
    ofSetFrameRate(30);
	//ofSetVerticalSync(true);
	ofBackground(0);
    ofHideCursor();
	
	shader_blur.load("shadersES2/shader_blur");
	
	//ofSetLogLevel(OF_LOG_VERBOSE);
	
    
    /**MIDIBIX***/
    
	// print input ports to console
	midiIn.listInPorts();
	
	// open port by number (you may need to change this)
	midiIn.openPort(1);
	//midiIn.openPort("IAC Pure Data In");	// by name
	//midiIn.openVirtualPort("ofxMidiIn Input"); // open a virtual port
	
	// don't ignore sysex, timing, & active sense messages,
	// these are ignored by default
	midiIn.ignoreTypes(false, false, false);
	
	// add ofApp as a listener
	midiIn.addListener(this);
	
	// print received messages to the console
	midiIn.setVerbose(true);
    
    aspect_fix_fbo.allocate(width,height);
	
	aspect_fix_fbo.begin();
	ofClear(0,0,0,255);
	aspect_fix_fbo.end();
   
    
    /*******/
    
    
     
    
    
    //pass in the settings and it will start the camera
	if(inputswitch==0){
		omx_settings(); 
		videoGrabber.setup(settings);
		
		for(int i=0;i<fbob;i++){
        
			pastFrames_img[i].allocate(width,height,OF_IMAGE_COLOR_ALPHA);
        
		}//endifor
		
	}//endif
	
	if(inputswitch==1){
		cam1.setVerbose(true);
		cam1.listDevices();
		cam1.setDesiredFrameRate(30);
		cam1.setDeviceID(0);
		cam1.initGrabber(width,height);
		
		for(int i=0;i<fbob;i++){
        
			pastFrames_img[i].allocate(width,height,OF_IMAGE_COLOR);
        
		}//endifor
		
	}//endif
	
    
    time_displaced_texture.allocate(width,height,GL_RGB);
    
    oscillator_texture2.allocate(width,height,GL_RGB);
     
    video_data= new unsigned char [width*height*3];
    displacement_data= new unsigned char [width*height*3];
    
    osc0_noise_image.allocate(width,height, OF_IMAGE_GRAYSCALE);
    osc1_noise_image.allocate(width,height, OF_IMAGE_GRAYSCALE);
    
    for(int i=0;i<p_lock_number;i++){
        
        for(int j=0;j<p_lock_size;j++){
            
            p_lock[i][j]=0;
            
        }//endplocksize
    
    }//endplocknumber
}

//--------------------------------------------------------------
void ofApp::update() {

    midibiz();
    
    
    
    for(int i=0;i<p_lock_number;i++){
        p_lock_smoothed[i]=p_lock[i][p_lock_increment]*(1.0f-p_lock_smooth)+p_lock_smoothed[i]*p_lock_smooth;
        
        if(abs(p_lock_smoothed[i])<.05){p_lock_smoothed[i]=0;}
        
    }
	
	
	float d_freeze_upper=int(fbob*(1.0-p_lock_smoothed[6]));
	float d_freeze_lower=ofClamp(int(fbob*(p_lock_smoothed[14])),0.0,freeze_upper);	
		
	
	
	
    
    if( (framedelayoffset<=d_freeze_upper) && (framedelayoffset>=d_freeze_lower) ){
		
		if(inputswitch==0){
			omx_updates();
			picapture_pixels=videoGrabber.getPixels();
			picapture_pixels.resize(width,height);
			//picapture_image.setFromPixels(picapture_pixels);
	
			pastFrames_img[abs(fbob-framedelayoffset)-1].setFromPixels(picapture_pixels);
		}
	
		if(inputswitch==1){
			cam1.update();
			
			if(hdmi_aspect_ratio_switch==0){
				pastFrames_img[abs(fbob-framedelayoffset)-1].setFromPixels(cam1.getPixels());
			}
			
			if(hdmi_aspect_ratio_switch==1){
				
				//aspect_fix_fbo.begin();
				//cam1.draw(0,0,426,240);
				//aspect_fix_fbo.end();
		
		        ofPixels aspect_fix_pixels;
		        aspect_fix_pixels=cam1.getPixels();
		        //aspect_fix_pixels.resize(426,240);
		        aspect_fix_pixels.resize(320,180);
		        
				pastFrames_img[abs(fbob-framedelayoffset)-1].setFromPixels(aspect_fix_pixels);
			}
			
			
		}
		
	}
    
  
    
	time_displace();
   
  
	
  
  
}

//----------------------------------------------------------------------
void ofApp:: time_displace(){
	
	//scaling coefficents
	float osc_frequency_scale=width/128.0f;
    float osc_amp_scale=2.0;
    float osc_rate_adder_scale=.3;
    float rgb_offset_scale=50;
	
	
	
	
	//dummy variables for p locks etc
	float d_time_length=fbob*(1.0f-p_lock_smoothed[7]);
	float d_osc0_rate_adder=p_lock_smoothed[0]*osc_rate_adder_scale;
	float d_osc0_frequency=p_lock_smoothed[1]*osc_frequency_scale;
	float d_osc1_rate_adder=p_lock_smoothed[2]*osc_rate_adder_scale;
	float d_osc1_frequency=p_lock_smoothed[3]*osc_frequency_scale;
	float d_red_offset=p_lock_smoothed[4]*rgb_offset_scale;
	float d_green_offset=p_lock_smoothed[5]*rgb_offset_scale;
	//6
	//7
	
	float d_osc0_amp=p_lock_smoothed[8]*osc_amp_scale;
	float d_osc0_angle=p_lock_smoothed[9];
	float d_osc1_amp=p_lock_smoothed[10]*osc_amp_scale;
	float d_osc1_angle=p_lock_smoothed[11];
	//no 12 yet??
	float d_blue_offset=p_lock_smoothed[13]*rgb_offset_scale;
	
	
	//increment (or don't) the oscillator rates
	if(osc_freeze==0){
        //osc0_rate+=osc0_rate_adder*osc_rate_adder_scale;
        //osc1_rate+=osc1_rate_adder*osc_rate_adder_scale;
        
        osc0_rate+=d_osc0_rate_adder;
        osc1_rate+=d_osc1_rate_adder;
    }
	
	
	
	 //scan through the buffers and compile a pixel thing with all the images
    
    
    
    //float time_length=fbob*(1.0f-c8);
    
    for(int i=0;i<width;i++) {
        //i is x
        for(int j=0;j<height;j++){
            //j is y
            
            int index=i+j*width;
            
            //normalize coordinates between 0-1
            float x_normalized=i/float(width/2);
            float y_normalized=j/float(height/2);

            
            //for radial oscillators
            float x_centered=(i-float(width/2))/float(width/2);
            float y_centered=(j-float(height/2))/float(height/2);
            
            ofVec2f centered_coords;
            centered_coords.set(x_centered,y_centered);
            
            
            
            
            //osc0
        
            
            //dunno if this is going to work or if the whole converting to chars thing is making shit too stepped
            //float osc_arg_smooth=.75;
            
            float osc0_dummy_arg=0.0;
            //testing out rotations
            
            //osc0_axis=rotation(ofVec2f(x_normalized,y_normalized),osc0_angle*3.14);
            osc0_axis.set(x_normalized,y_normalized);
            //centered_coords=rotation(centered_coords,osc0_angle);
            
            if(osc0_radial==0){
                osc0_dummy_arg=osc0_axis.x*(1.0f-d_osc0_angle)+osc0_axis.y*(d_osc0_angle);
            }
            
            //radiating circular
            if(osc0_radial==1){
                osc0_dummy_arg=sqrt(x_centered*x_centered+y_centered*y_centered);
            }
            
            //radiating parabolic
            if(osc0_radial==2){
                osc0_dummy_arg=sqrt(abs(x_centered*x_centered-y_centered*y_centered));
                //osc0_dummy_arg=sqrt(abs(osc0_axis.x*osc0_axis.x-osc0_axis.y*osc0_axis.y));
            }
            
            //radiating cubic
            if(osc0_radial==3){
                osc0_dummy_arg=sqrt(abs(x_centered*x_centered*x_centered-y_centered*y_centered*y_centered));
                //osc0_dummy_arg=sqrt(abs(centered_coords.x*centered_coords.x*centered_coords.x-centered_coords.y*centered_coords.y*centered_coords.y));
            }
            
            osc0_argument=osc0_dummy_arg
                         *d_osc0_frequency
                         *TWO_PI+
                         osc0_rate;
            
            
            float osc0_displace=d_osc0_amp*time_osc(osc0_argument,osc0_shape,0,i,j);
            
            
            
            
            
            
            
            float osc1_dummy_arg=0.0;
            
            //testing out rotations
            
            //osc1_axis=rotation(ofVec2f(x_normalized,y_normalized),osc1_angle*3.14);
            osc1_axis.set(x_normalized,y_normalized);
            //centered_coords=rotation(centered_coords,osc1_angle);
            
            if(osc1_radial==0){
                osc1_dummy_arg=osc1_axis.y*(1.0f-d_osc1_angle)+osc1_axis.x*(d_osc1_angle);
            }
            
            //radiating circular
            if(osc1_radial==1){
                osc1_dummy_arg=sqrt(x_centered*x_centered+y_centered*y_centered);
            }
            
            //radiating parabolic
            if(osc1_radial==2){
                osc1_dummy_arg=sqrt(abs(x_centered*x_centered-y_centered*y_centered));
                //osc1_dummy_arg=sqrt(abs(osc1_axis.x*osc1_axis.x-osc1_axis.y*osc1_axis.y));
            }
            
            //radiating cubic
            if(osc1_radial==3){
                osc1_dummy_arg=sqrt(abs(x_centered*x_centered*x_centered-y_centered*y_centered*y_centered));
                //osc1_dummy_arg=sqrt(abs(centered_coords.x*centered_coords.x*centered_coords.x-centered_coords.y*centered_coords.y*centered_coords.y));
            }
            
            osc1_argument=osc1_dummy_arg
            *d_osc1_frequency
            *TWO_PI+
            osc1_rate;
            
            
            
            
            
            
            float osc1_displace=d_osc1_amp*time_osc(osc1_argument,osc1_shape,1,i,j);
            
            //cout<<"osc1_displace = "<<osc1_displace<<" osc1_argument ="<<osc1_argument<<endl;
            
            
            
  
            
            
            float displacement_amount_normalized=0;
            
            
            //if nothing clicked add and clamp
            if(overflow_switch==0){
                displacement_amount_normalized=(osc1_displace+osc0_displace)/2.0f;
            
                displacement_amount_normalized=ofClamp(displacement_amount_normalized,0.0,1.0);
            }
            //do some overflows stuff here
            
            
            //add and wraparound
            if(overflow_switch==1){
                displacement_amount_normalized=(osc1_displace+osc0_displace);
                displacement_amount_normalized=fmod(abs(displacement_amount_normalized),1.0);
            }
            
            //add and foldover
            if(overflow_switch==2){
                displacement_amount_normalized=(osc1_displace+osc0_displace);
                if(displacement_amount_normalized>1.0){
                    displacement_amount_normalized=1.0-fmod(abs(displacement_amount_normalized),1.0);
                }
            }
            
            if(overflow_switch==3){
                displacement_amount_normalized=(osc1_displace*osc0_displace);
                if(displacement_amount_normalized>1.0){
                    displacement_amount_normalized=1.0-fmod(abs(displacement_amount_normalized),1.0);
                }
            }
            
            if(overflow_switch==4){
                osc1_displace=ofMap(osc1_displace,0,2.0,0,6.18);
                osc0_displace=ofMap(osc0_displace,0,2.0,0,6.18);
                displacement_amount_normalized=(sin(osc1_displace+osc0_displace));
                if(displacement_amount_normalized>1.0){
                    displacement_amount_normalized=1.0-fmod(abs(displacement_amount_normalized),1.0);
                }
                if(displacement_amount_normalized<0){
                    displacement_amount_normalized=1.0-fmod(abs(displacement_amount_normalized),1.0);
                }
            }
            
            if(overflow_switch==5){
                float osc0_displace_dummy=osc0_displace;
                osc0_displace=d_osc0_amp*time_osc(osc0_argument+6.18*osc1_displace,osc0_shape,0,i,j);
                osc1_displace=d_osc1_amp*time_osc(osc1_argument+6.18*osc0_displace_dummy,osc1_shape,1,i,j);
                displacement_amount_normalized=(osc0_displace+osc1_displace)/2.0f;
                
                if(displacement_amount_normalized>1.0){
                    displacement_amount_normalized=1.0-fmod(abs(displacement_amount_normalized),1.0);
                    //displacement_amount_normalized=1;
                }
                if(displacement_amount_normalized<0){
                    displacement_amount_normalized=abs(displacement_amount_normalized);
                    
                }
            }
            
            if(overflow_switch==6){
               
                displacement_amount_normalized=(osc0_displace/(osc1_displace+.001f));
                
                if(displacement_amount_normalized>1.0){
                    displacement_amount_normalized=1.0-fmod(abs(displacement_amount_normalized),1.0);
                    //displacement_amount_normalized=1;
                }
                if(displacement_amount_normalized<0){
                    displacement_amount_normalized=abs(displacement_amount_normalized);
                    
                }
            }
            
            //int displacement_amount=int(displacement_amount_normalized*time_length);
            int displacement_amount=int(displacement_amount_normalized*d_time_length);
            int time_pointer=abs(fbob-framedelayoffset+displacement_amount)%fbob;
            
            //img version
            //unsigned char* past_pixel_data=pastFrames_img[time_pointer].getPixels().getData();
            
            
            //seperate RGB displacement version
           
           
            
            int red_index=(time_pointer+int(d_red_offset))%fbob;
            int green_index=(time_pointer+int(d_green_offset))%fbob;
            int blue_index=(time_pointer+int(d_blue_offset))%fbob;
            
            unsigned char* R_past_pixel_data=pastFrames_img[red_index].getPixels().getData();
            unsigned char* G_past_pixel_data=pastFrames_img[green_index].getPixels().getData();
            unsigned char* B_past_pixel_data=pastFrames_img[blue_index].getPixels().getData();
            
           
            //pixelsversion
            //unsigned char* past_pixel_data=pastFrames_pix[time_pointer].getData();
        
            //rgb offsetting as well...
            
            
            
        
            
        
            //if(red_index<0){red_index=0;}
            
            //ifRGB
            if(inputswitch==1){
                //video_data[index*3]=past_pixel_data[index*3];
                //video_data[index*3+1]=past_pixel_data[index*3+1];
                //video_data[index*3+2]=past_pixel_data[index*3+2];
                
                
                
                
				video_data[index*3]=R_past_pixel_data[index*3];
				video_data[index*3+1]=G_past_pixel_data[index*3+1];
				video_data[index*3+2]=B_past_pixel_data[index*3+2];
				
            }
            //ifRGBA
            
            if(inputswitch==0){
                //video_data[index*3]=R_past_pixel_data[index*4];
                //video_data[index*3+1]=G_past_pixel_data[index*4+1];
                //video_data[index*3+2]=B_past_pixel_data[index*4+2];
            }
            
            if(osc_draw_switch==1){ 
				displacement_data[index*3]=
				displacement_data[index*3+1]=
				displacement_data[index*3+2]=
				int(displacement_amount_normalized*255);
			}
        }
    }
    
   
    
    //osc0_rate=fmod(osc0_rate,6.18);
    //osc1_rate=fmod(osc0_rate,6.18);
    
    time_displaced_texture.loadData(video_data,width,height,GL_RGB);
    
    if(osc_draw_switch==1){ 
     oscillator_texture2.loadData(displacement_data,width,height,GL_RGB);
	}
	
	
	 if(osc0_shape==3){
		osc0_noise_image=perlin_noise(osc0_rate,d_osc0_frequency,osc0_noise_image);
		
	}	
	
	if(osc1_shape==3){
		osc1_noise_image=perlin_noise(osc1_rate,d_osc1_frequency,osc1_noise_image);
    }
}

//---------------____-------__-_------___-_-_-_-_-__--_-_
//---------------____-------__-_------___-_-_-_-_-__--_-_

float ofApp:: time_osc(float frequency,int shape,int osc_id,int x, int y){
    float osc=0.0;
    
    
    //osc=(1.0f+sin(frequency))/2.0f;
    
    //triangle
    if(shape==0){
        osc=(abs(ofMap(fmod(abs(frequency)/6.18,1.0),0.0,1.0,-1.0,1.0)));
    }
    
    if(shape==1){
        osc=ofMap(fmod(frequency/6.18,1.0),-1.0,1.0,0.0,1.0);
        //cout<<osc<<endl;
    }
    
    //square
    if(shape==2){
        osc=ofSign(sin(frequency));
        //osc=(sin(frequency)+1.0f)/2.0f;
    }
    //osc=sin(frequency);
    
    if(shape==3){
        if(osc_id==0){
            //osc0
            unsigned char* noise_data=osc0_noise_image.getPixels().getData();
            osc=float(noise_data[x+width*y]/255.0f);
        }
        
        if(osc_id==1){
            //osc1
            unsigned char* noise_data=osc1_noise_image.getPixels().getData();
            osc=float(noise_data[x+width*y]/255.0f);
            //cout<<"hello"<<endl;
        }
    }
    
    return osc;
    
}

//8--_----_--__----_-___----_


ofVec2f ofApp:: rotation(ofVec2f coord,float theta){
    ofVec2f center_coord=ofVec2f(coord.x-width/2,coord.y-height/2);
    ofVec2f rotate_coord=ofVec2f(0,0);
    float rotate=abs(coord.x+coord.y)/2*width;
    coord.x=rotate+coord.x;
    coord.y=rotate+coord.y;
    rotate_coord.x=center_coord.x*cos(theta)-center_coord.y*sin(theta);
    rotate_coord.y=center_coord.x*sin(theta)+center_coord.y*cos(theta);
    
    
    rotate_coord=rotate_coord+ofVec2f(width/2,height/2);
    
    return rotate_coord;
    
    
}//endrotate
//+---------------------------------------------------------------

ofImage ofApp::perlin_noise(float theta, float resolution, ofImage noise_image){
    
    //float noiseScale = ofMap(mouseX, 0, ofGetWidth(), 0, 0.1);
    //float noiseVel = ofGetElapsedTimef();
    resolution=resolution*.01;
    theta=theta*.1;
    ofPixels & pixels = noise_image.getPixels();
    int w = noise_image.getWidth();
    int h = noise_image.getHeight();
    for(int y=0; y<h; y++) {
        for(int x=0; x<w; x++) {
            int i = y * w + x;
            float noiseVelue = ofNoise(x * resolution, y * resolution, theta);
            pixels[i] = 255 * noiseVelue;
        }
    }
    
    
    noise_image.update();
    return noise_image;
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(0);
    
    
    shader_blur.begin();
    if(hdmi_aspect_ratio_switch==0){
		time_displaced_texture.draw(0,0,720,480);
	}
	
	
	if(hdmi_aspect_ratio_switch==1){
		time_displaced_texture.draw(0,0,720*1.34,480*1.34);
	}
	
	shader_blur.setUniform1f("blur_amount", p_lock_smoothed[12]);
	
	shader_blur.end();
	
	
    if(osc_draw_switch==1){
		oscillator_texture2.draw(0,0,width/2,height/2);
    }
    
    
    
    incIndex(); // increment framecount and framedelayoffset eeettt
    
    //p_lock biz
	 if(p_lock_switch==1){
        p_lock_increment++;
        p_lock_increment=p_lock_increment%p_lock_size;
    }
   
    
    //ofSetColor(0);
    //string msg="fps="+ofToString(ofGetFrameRate(),2);//+" clear switch"+ofToString(clear_switch,5);//+" z="+ofToString(az,5);
    //ofDrawBitmapString(msg,10,10);
}



//--------------------------------------------------------------
void ofApp::exit() {
	
	// clean up
	midiIn.closePort();
	midiIn.removeListener(this);
}

//----------------------------------------------------------------
void ofApp::omx_settings(){
	
	settings.sensorWidth = 640;
    settings.sensorHeight = 480;
    settings.framerate = 30;
    settings.enableTexture = true;
    settings.sensorMode=7;
    
    settings.whiteBalance ="Off";
    settings.exposurePreset ="Off";
    settings.whiteBalanceGainR = 1.0;
    settings.whiteBalanceGainB = 1.0;
    
    settings.enablePixels = true;
	
	
	}
//------------------------------------------------------------

void ofApp::omx_updates(){
	
		videoGrabber.setSharpness(50);
		videoGrabber.setBrightness(40);
		videoGrabber.setContrast(100);
		videoGrabber.setSaturation(0);
	
	}


//--------------------------------------------------------------
void ofApp::newMidiMessage(ofxMidiMessage& msg) {

	// add the latest message to the message queue
	midiMessages.push_back(msg);

	// remove any old messages if we have too many
	while(midiMessages.size() > maxMessages) {
		midiMessages.erase(midiMessages.begin());
	}
}


//_____----_-_-_-______---__---_--_----____--_-__-_-


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {

    
    

  if(key=='a'){fix+=.01;cout<<fix<<endl;}
  if(key=='z'){fix-=.01;cout<<fix<<endl;}
 
    
    
    
    
   
}


//---------------------
//---------------------
void ofApp:: midibiz(){
    
    for(unsigned int i = 0; i < midiMessages.size(); ++i) {
        
        ofxMidiMessage &message = midiMessages[i];
        
        
        //cout << "envcount="<< envcount<< endl;
        
        if(message.status < MIDI_SYSEX) {
            //text << "chan: " << message.channel;
            if(message.status == MIDI_CONTROL_CHANGE) {
                //  cout << "message.control"<< message.control<< endl;
                //  cout << "message.value"<< message.value<< endl;
                
               
                
                //turn on and off the visible oscillator
                if(message.control==46){
                    osc_draw_switch=int(message.value/127.0f);
                }
                
                
                  if(message.control==60){
                    if(message.value==127){
                        p_lock_switch=1;
                        // p_lock_increment=0;
                        
                        for(int i=0;i<p_lock_number;i++){
								p_lock_smoothed[i]=0;
								for(int j=0;j<p_lock_size;j++){
                                
									p_lock[i][j]=p_lock[i][p_lock_increment];
                                
								}//endplocksize
                            
							}//endplocknumber
                        
                    }
                    
                    if(message.value==0){
                        p_lock_switch=0;
                    }
                    
                }
                
                if(message.control==62){
					if(message.value==127){
						//fb0_h_invert=TRUE;
						hdmi_aspect_ratio_switch=TRUE;
					}
					
					if(message.value==0){
						//fb0_h_invert=FALSE;
						hdmi_aspect_ratio_switch=FALSE;
					}
                }
                
                if(message.control==61){
                    if(message.value==127){
                        osc_freeze=1;
                    }
                    if(message.value==0){
                        osc_freeze=0;
                    }
                }
                
                //overflow controls
                if(message.control==43){
                    
                    if(message.value==127){
                        overflow_switch=1;
                    }
                    
                    if(message.value==0){
                        overflow_switch=0;
                    }
                }
                
                if(message.control==44){
                    if(message.value==127){
                        overflow_switch=2;
                    }
                    
                    if(message.value==0){
                        overflow_switch=0;
                    }
                }
                
                if(message.control==42){
                    if(message.value==127){
                        overflow_switch=3;
                    }
                    
                    if(message.value==0){
                        overflow_switch=0;
                    }
                }
                
                if(message.control==41){
                    if(message.value==127){
                        overflow_switch=4;
                    }
                    
                    if(message.value==0){
                        overflow_switch=0;
                    }
                }
                if(message.control==45){
                    if(message.value==127){
                        overflow_switch=5;
                    }
                    
                    if(message.value==0){
                        overflow_switch=0;
                    }
                }
                
                //osc0 shape
                if(message.control==32){
                    if(message.value==127){
                        osc0_shape=1;
                    }
                    
                    if(message.value==0){
                        osc0_shape=0;
                    }
                }
                
                if(message.control==48){
                    if(message.value==127){
                        osc0_shape=2;
                    }
                    
                    if(message.value==0){
                        osc0_shape=0;
                    }
                }
                
                if(message.control==64){
                    if(message.value==127){
                        osc0_shape=3;
                    }
                    
                    if(message.value==0){
                        osc0_shape=0;
                    }
                }
                
                //osc0_radial
                if(message.control==33){
                    if(message.value==127){
                        osc0_radial=1;
                    }
                    
                    if(message.value==0){
                        osc0_radial=0;
                    }
                }
                
                if(message.control==49){
                    if(message.value==127){
                        osc0_radial=2;
                    }
                    
                    if(message.value==0){
                        osc0_radial=0;
                    }
                }
                
                if(message.control==65){
                    if(message.value==127){
                        osc0_radial=3;
                    }
                    
                    if(message.value==0){
                        osc0_radial=0;
                    }
                }
                
                //osc1 shape
                if(message.control==34){
                    if(message.value==127){
                        osc1_shape=1;
                    }
                    
                    if(message.value==0){
                        osc1_shape=0;
                    }
                }
                
                if(message.control==50){
                    if(message.value==127){
                        osc1_shape=2;
                    }
                    
                    if(message.value==0){
                        osc1_shape=0;
                    }
                }
                
                if(message.control==66){
                    if(message.value==127){
                        osc1_shape=3;
                    }
                    
                    if(message.value==0){
                        osc1_shape=0;
                    }
                }
                
                //osc1_radial
                if(message.control==35){
                    if(message.value==127){
                        osc1_radial=1;
                    }
                    
                    if(message.value==0){
                        osc1_radial=0;
                    }
                }
                
                if(message.control==51){
                    if(message.value==127){
                        osc1_radial=2;
                    }
                    
                    if(message.value==0){
                        osc1_radial=0;
                    }
                }
                
                if(message.control==67){
                    if(message.value==127){
                        osc1_radial=3;
                    }
                    
                    if(message.value==0){
                        osc1_radial=0;
                    }
                }
                
                
                //nanokontrol2 controls
                if(message.control==16){
                      c1=(message.value-63.0)/63.0;
                    osc0_rate_adder=(message.value-63.0)/63.0;
                    p_lock[0][p_lock_increment]=(message.value-63.0f)/63.0f;
                    //c1=(message.value)/127.00;
                    
                }
                
                
                if(message.control==17){
                     //c2=(message.value-63.0)/63.0;
                    c2=(message.value)/127.00;
                    osc0_frequency=(message.value)/127.00;
                    p_lock[1][p_lock_increment]=(message.value)/127.00f;
                }
                
                
                if(message.control==18){
                    c3=(message.value-63.0)/63.00;
                    osc1_rate_adder=(message.value-63.0)/63.0;
                    p_lock[2][p_lock_increment]=(message.value-63.0f)/63.0f;
                    
                    //  c3=(message.value)/127.00;
                }
                
                
                if(message.control==19){
                    //c4=(message.value-63.0)/63.00;
                    c4=(message.value)/127.00;
                    osc1_frequency=(message.value)/127.00;
                    p_lock[3][p_lock_increment]=(message.value)/127.0f;
                    
                }
                
                
                if(message.control==20){
                    //c5=(message.value-63.0)/63.00;
                    c5=(message.value)/127.00;
                    red_offset=(message.value)/127.00;
                    p_lock[4][p_lock_increment]=(message.value)/127.0f;
                }
                
                
                if(message.control==21){
                    //c6=(message.value-63.0)/63.0;
                    c6=(message.value)/127.00;
                    green_offset=(message.value)/127.00;
                    p_lock[5][p_lock_increment]=(message.value)/127.0f;
                }
                
                
                if(message.control==22){
                    //c7=(message.value-63.0)/63.0;
                    c7=(message.value)/127.00;
                    p_lock[6][p_lock_increment]=(message.value)/127.0f;
                   
                }
                
                
                if(message.control==23){
                    // c8=(message.value-63.0)/63.00;
                    c8=(message.value)/127.0;
                    p_lock[7][p_lock_increment]=(message.value)/127.0f;
                }
                
                if(message.control==120){
                    //c9=(message.value-63.0)/63.0;
                    c9=(message.value)/127.00;
                    osc0_amp=(message.value)/127.00;
                    p_lock[8][p_lock_increment]=(message.value)/127.00;
                }
                
                if(message.control==121){
                    //c10=(message.value-63.0)/63.0;
                    c10=(message.value)/127.00;
                    osc0_angle=(message.value)/127.00;
                    p_lock[9][p_lock_increment]=(message.value)/127.00f;
                }
                
                if(message.control==122){
                    //c11=(message.value-63.0)/63.00;
                    c11=(message.value)/127.00;
                    osc1_amp=(message.value)/127.00;
					p_lock[10][p_lock_increment]=(message.value)/127.00f;
                }
                
                
                if(message.control==123){
                    // c12=(message.value-63.0)/63.00;
                    c12=(message.value)/127.00;
                    osc1_angle=(message.value)/127.00;
                    p_lock[11][p_lock_increment]=(message.value)/127.00f;
                }
                
                if(message.control==124){
                    c13=(message.value-63.0)/63.00;
                    //c13=(message.value)/127.00;
                    p_lock[12][p_lock_increment]=(message.value)/127.0f; 
                   
                   
                   

                    
                }
                
                if(message.control==125){
                    c14=(message.value-63.0)/63.0;
                    blue_offset=(message.value)/127.00;
                   // c14=(message.value)/127.00;
                   p_lock[13][p_lock_increment]=(message.value)/127.0f;
                }
                
                if(message.control==126){
                    //c15=(message.value-63.0)/63.0;
                    c15=(message.value)/127.00;
                    
                    p_lock[14][p_lock_increment]=(message.value)/127.0f;
                   
                    
                    
                    
                }
                
                if(message.control==127){
                    //c16=(message.value-63.0)/63.00;
                    c16=(message.value)/127.0;
                    p_lock[15][p_lock_increment]=(message.value)/127.0f;
                }
                
                
               
                
                if(message.control==58){
                   
						
					if(message.value==127){
                        for(int i=0;i<p_lock_number;i++){
                            
                            for(int j=0;j<p_lock_size;j++){
                                
                                p_lock[i][j]=0;
                                
                            }//endplocksize
                            
                        }//endplocknumber
                    
                }
                    
                    
                    red_offset=0;
                    green_offset=0;
                    blue_offset=0;
                    
                    freeze_upper=fbob;
                    freeze_lower=0;
                    
                }

                
                
                
              
                
                //text << "\tctl: " << message.control;
                //ofDrawRectangle(x + ofGetWidth()*0.2, y + 12,
                //	ofMap(message.control, 0, 127, 0, ofGetWidth()*0.2), 10);
            }
            
            
            //text << " "; // pad for delta print
        }//
        
        
    }
    
    
    /******* endmidimessagesbiz*********/
    
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    
}





//--------------------------------------------------------------


