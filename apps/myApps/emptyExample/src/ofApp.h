#pragma once

#include "ofMain.h"

class ofApp : public ofBaseApp{
	public:
		void setup();
		void update();
		void draw();
		void exit();
		void urlResponse(ofHttpResponse & response);
		
		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y);
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);

	private:
		struct DayForecast {
			std::string day;
			std::string condition;
			int code = 0;
			int high = 0;
			int low = 0;
		};

		void searchCity();
		void requestWeather(double latitude, double longitude);
		void drawWeatherIcon(float x, float y, float size, int code, bool isDay);
		void drawText(const std::string & text, float x, float y, int size, ofColor color);
		std::string weatherDescription(int code) const;
		std::string encodeUrl(const std::string & value) const;

		std::string cityQuery = "London";
		std::string cityName = "London";
		std::string regionName = "United Kingdom";
		std::string statusMessage = "Connecting to live weather...";
		std::string updatedTime;
		bool searchActive = false;
		bool loading = true;
		bool isDay = true;
		int weatherCode = 2;
		int temperature = 18;
		int feelsLike = 17;
		int humidity = 62;
		int windSpeed = 12;
		int precipitation = 20;
		int highToday = 21;
		int lowToday = 13;
		int uvIndex = 4;
		int geoRequestId = -1;
		int weatherRequestId = -1;
		std::vector<DayForecast> forecast;
		std::vector<int> hourlyTemperatures;
		std::vector<std::string> hourlyLabels;
		ofTrueTypeFont displayFont;
		ofTrueTypeFont textFont;
		bool fontsReady = false;
};
