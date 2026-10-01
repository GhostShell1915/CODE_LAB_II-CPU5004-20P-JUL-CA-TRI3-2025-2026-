#include "ofApp.h"
#include <cctype>

namespace {
const ofColor background(242, 243, 235);
const ofColor ink(31, 55, 45);
const ofColor muted(112, 126, 115);
const ofColor green(50, 111, 83);
const ofColor lime(208, 224, 143);
const ofColor orange(232, 133, 67);
const ofColor pale(255, 255, 249);
}

//--------------------------------------------------------------
void ofApp::setup(){
	ofSetWindowTitle("Weather / Open Frameworks");
	ofSetFrameRate(60);
	fontsReady = displayFont.load("C:/Windows/Fonts/seguisb.ttf", 58, true, true)
		&& textFont.load("C:/Windows/Fonts/segoeui.ttf", 16, true, true);
	forecast = {
		{"Today", "Partly cloudy", 2, 21, 13},
		{"Tue", "Light rain", 61, 18, 12},
		{"Wed", "Cloudy", 3, 17, 11},
		{"Thu", "Clear sky", 0, 20, 12},
		{"Fri", "Showers", 80, 16, 10}
	};
	hourlyTemperatures = {18, 19, 20, 20, 19, 17};
	hourlyLabels = {"12:00", "14:00", "16:00", "18:00", "20:00", "22:00"};
	ofAddListener(ofURLResponseEvent(), this, &ofApp::urlResponse);
	searchCity();
}

//--------------------------------------------------------------
void ofApp::update(){
}

//--------------------------------------------------------------
void ofApp::draw(){
	const float w = ofGetWidth();
	const float h = ofGetHeight();
	const float margin = 56;
	ofBackground(background);
	ofSetColor(ink);
	ofSetLineWidth(1);

	// Header and city search
	ofSetColor(green);
	ofDrawCircle(margin + 12, 42, 12);
	ofSetColor(lime);
	ofDrawCircle(margin + 12, 42, 4);
	drawText("FIELD NOTES", margin + 34, 48, 16, ink);
	drawText("WEATHER / 01", margin + 34, 67, 11, muted);

	const float searchX = w - margin - 316;
	ofSetColor(pale);
	ofDrawRectangle(searchX, 27, 316, 42);
	ofSetColor(searchActive ? orange : ofColor(208, 213, 202));
	ofNoFill();
	ofDrawRectangle(searchX, 27, 316, 42);
	ofFill();
	ofSetColor(muted);
	ofDrawCircle(searchX + 18, 45, 6);
	ofSetColor(pale);
	ofDrawCircle(searchX + 18, 45, 4);
	ofSetColor(muted);
	ofDrawLine(searchX + 22, 49, searchX + 27, 54);
	drawText(cityQuery.empty() ? "Search a city" : cityQuery, searchX + 35, 52, 14, searchActive ? ink : muted);
	ofSetColor(green);
	ofDrawRectangle(searchX + 270, 27, 46, 42);
	ofSetColor(pale);
	drawText("GO", searchX + 281, 53, 13, pale);
	ofSetColor(ofColor(216, 220, 209));
	ofDrawLine(margin, 88, w - margin, 88);

	// Current conditions
	drawText("CURRENT CONDITIONS", margin, 125, 12, green);
	drawText(cityName, margin, 161, 25, ink);
	drawText(regionName, margin, 184, 13, muted);
	if (loading) {
		drawText("UPDATING LIVE DATA", margin + 250, 125, 11, orange);
	} else if (!statusMessage.empty()) {
		drawText(statusMessage, margin + 250, 125, 11, orange);
	}

	const float heroY = 229;
	drawText(ofToString(temperature), margin - 3, heroY + 59, 74, ink);
	drawText("°", margin + 106, heroY + 26, 31, orange);
	drawText("C", margin + 132, heroY + 27, 20, muted);
	drawText(weatherDescription(weatherCode), margin, heroY + 88, 17, ink);
	drawText("Feels like " + ofToString(feelsLike) + "°", margin, heroY + 113, 13, muted);
	drawText("H " + ofToString(highToday) + "°     L " + ofToString(lowToday) + "°", margin, heroY + 140, 13, green);

	const float iconX = w * 0.39f;
	drawWeatherIcon(iconX, heroY + 42, 104, weatherCode, isDay);
	drawText(isDay ? "DAYLIGHT" : "AFTER DARK", iconX - 45, heroY + 132, 11, muted);

	// Conditions at a glance
	const float statsX = w * 0.57f;
	const float statsW = w - margin - statsX;
	const float statGap = 12;
	const float statW = (statsW - statGap) / 2;
	const float statY = 201;
	const float statH = 78;
	const auto drawStat = [&](float x, float y, const std::string & label, const std::string & value, const std::string & note, ofColor marker) {
		ofSetColor(pale);
		ofDrawRectangle(x, y, statW, statH);
		ofSetColor(marker);
		ofDrawRectangle(x, y, 3, statH);
		drawText(label, x + 16, y + 23, 11, muted);
		drawText(value, x + 16, y + 50, 22, ink);
		drawText(note, x + statW - 79, y + 49, 11, muted);
	};
	drawStat(statsX, statY, "WIND", ofToString(windSpeed) + " km/h", "NW", green);
	drawStat(statsX + statW + statGap, statY, "HUMIDITY", ofToString(humidity) + "%", "AIR", orange);
	drawStat(statsX, statY + statH + 12, "RAIN CHANCE", ofToString(precipitation) + "%", "TODAY", orange);
	drawStat(statsX + statW + statGap, statY + statH + 12, "UV INDEX", ofToString(uvIndex), "MODERATE", green);

	// Forecast band
	const float bandY = h * 0.59f;
	const float bandH = h - bandY - 42;
	ofSetColor(ink);
	ofDrawRectangle(margin, bandY, w - margin * 2, bandH);
	drawText("THE NEXT HOURS", margin + 24, bandY + 30, 12, lime);
	drawText("5-DAY OUTLOOK", w * 0.60f, bandY + 30, 12, lime);
	ofSetColor(ofColor(86, 107, 91));
	ofDrawLine(w * 0.57f, bandY + 18, w * 0.57f, bandY + bandH - 18);

	const float chartX = margin + 26;
	const float chartW = w * 0.49f - margin;
	const float chartTop = bandY + 68;
	const float chartBottom = bandY + bandH - 48;
	const int hoursCount = static_cast<int>(hourlyTemperatures.size());	
	if (hoursCount > 1) {
		std::vector<ofPoint> points;
		for (int i = 0; i < hoursCount; ++i) {
			const float px = chartX + (chartW - 30) * i / (hoursCount - 1);
			const float py = chartBottom - (hourlyTemperatures[i] - 10) * 5.0f;
			points.emplace_back(px, py);
			ofSetColor(ofColor(94, 116, 96));
			ofDrawLine(px, chartTop, px, chartBottom + 12);
			const std::string hourLabel = i < static_cast<int>(hourlyLabels.size()) ? hourlyLabels[i] : "";
			drawText(hourLabel, px - 16, bandY + bandH - 20, 10, ofColor(181, 196, 175));
			drawText(ofToString(hourlyTemperatures[i]) + "°", px - 10, py - 16, 11, pale);
		}
		ofSetColor(lime);
		for (std::size_t i = 1; i < points.size(); ++i) {
			ofDrawLine(points[i - 1], points[i]);
		}
		for (const auto & point : points) {
			ofDrawCircle(point, 3.5f);
		}
	}

	const float daysX = w * 0.60f;
	const float daysW = w - margin - daysX - 18;
	const float dayW = daysW / 5.0f;
	for (int i = 0; i < 5; ++i) {
		const float x = daysX + i * dayW;
		const DayForecast & day = forecast[std::min(static_cast<std::size_t>(i), forecast.size() - 1)];
		drawText(day.day, x, bandY + 66, 11, ofColor(196, 208, 191));
		drawWeatherIcon(x + dayW * 0.5f, bandY + 111, 30, day.code, true);
		drawText(ofToString(day.high) + "°", x, bandY + 159, 12, pale);
		drawText(ofToString(day.low) + "°", x, bandY + 181, 11, ofColor(166, 185, 164));
	}

	drawText(loading ? "Open-Meteo  /  fetching forecast" : "Open-Meteo  /  updated " + updatedTime,
		margin, h - 18, 10, muted);
	drawText("TYPE A CITY  +  ENTER", w - margin - 150, h - 18, 10, muted);
}

//--------------------------------------------------------------
void ofApp::exit(){
	ofRemoveListener(ofURLResponseEvent(), this, &ofApp::urlResponse);
}

//--------------------------------------------------------------
void ofApp::urlResponse(ofHttpResponse & response){
	if (response.request.name == "weather-geocode" && response.request.getId() == geoRequestId) {
		if (response.status < 200 || response.status >= 300) {
			loading = false;
			statusMessage = "Could not find that city";
			return;
		}
		const ofJson data = ofJson::parse(response.data.getText(), nullptr, false);
		if (data.is_discarded() || !data.contains("results") || data["results"].empty()) {
			loading = false;
			statusMessage = "City not found - try another";
			return;
		}
		const auto place = data["results"][0];
		cityName = place.value("name", cityQuery);
		regionName = place.value("admin1", place.value("country", ""));
		requestWeather(place.value("latitude", 0.0), place.value("longitude", 0.0));
		return;
	}

	if (response.request.name == "weather-forecast" && response.request.getId() == weatherRequestId) {
		loading = false;
		if (response.status < 200 || response.status >= 300) {
			statusMessage = "Weather service unavailable";
			return;
		}
		const ofJson data = ofJson::parse(response.data.getText(), nullptr, false);
		if (data.is_discarded() || !data.contains("current") || !data.contains("daily")) {
			statusMessage = "Weather data could not be read";
			return;
		}
		const auto current = data["current"];
		const auto daily = data["daily"];
		temperature = static_cast<int>(std::round(current.value("temperature_2m", 0.0)));
		feelsLike = static_cast<int>(std::round(current.value("apparent_temperature", 0.0)));
		humidity = current.value("relative_humidity_2m", 0);
		windSpeed = static_cast<int>(std::round(current.value("wind_speed_10m", 0.0)));
		weatherCode = current.value("weather_code", 0);
		isDay = current.value("is_day", 1) != 0;
		if (daily.contains("temperature_2m_max") && !daily["temperature_2m_max"].empty()) {
			highToday = static_cast<int>(std::round(daily["temperature_2m_max"][0].get<double>()));
			lowToday = static_cast<int>(std::round(daily["temperature_2m_min"][0].get<double>()));
		}
		if (daily.contains("uv_index_max") && !daily["uv_index_max"].empty()) {
			uvIndex = static_cast<int>(std::round(daily["uv_index_max"][0].get<double>()));
		}
		if (daily.contains("precipitation_probability_max") && !daily["precipitation_probability_max"].empty()) {
			precipitation = daily["precipitation_probability_max"][0].get<int>();
		}
		forecast.clear();
		if (daily.contains("time") && daily.contains("weather_code") && daily.contains("temperature_2m_max") && daily.contains("temperature_2m_min")) {
			const auto dates = daily["time"];
			for (std::size_t i = 0; i < dates.size() && i < 5; ++i) {
				std::string dayName = i == 0 ? "Today" : dates[i].get<std::string>().substr(5);
				if (i == 1) dayName = "Tomorrow";
				const int code = daily["weather_code"][i].get<int>();
				forecast.push_back({dayName, weatherDescription(code), code,
					static_cast<int>(std::round(daily["temperature_2m_max"][i].get<double>())),
					static_cast<int>(std::round(daily["temperature_2m_min"][i].get<double>()))});
			}
		}
		hourlyTemperatures.clear();
		hourlyLabels.clear();
		if (data.contains("hourly") && data["hourly"].contains("temperature_2m")) {
			const auto hourly = data["hourly"]["temperature_2m"];
			for (std::size_t i = 0; i < hourly.size() && i < 6; ++i) {
				hourlyTemperatures.push_back(static_cast<int>(std::round(hourly[i].get<double>())));
			}
			if (data["hourly"].contains("time")) {
				const auto times = data["hourly"]["time"];
				for (std::size_t i = 0; i < times.size() && i < 6; ++i) {
					const std::string timestamp = times[i].get<std::string>();
					hourlyLabels.push_back(timestamp.size() >= 16 ? timestamp.substr(11, 5) : "");
				}
			}
		}
		statusMessage.clear();
		updatedTime = ofGetTimestampString("%H:%M");
	}
}

//--------------------------------------------------------------
void ofApp::searchCity(){
	if (cityQuery.empty()) {
		statusMessage = "Enter a city name to search";
		return;
	}
	loading = true;
	statusMessage.clear();
	const std::string url = "https://geocoding-api.open-meteo.com/v1/search?name=" + encodeUrl(cityQuery) + "&count=1&language=en&format=json";
	geoRequestId = ofLoadURLAsync(url, "weather-geocode");
}

//--------------------------------------------------------------
void ofApp::requestWeather(double latitude, double longitude){
	const std::string url = "https://api.open-meteo.com/v1/forecast?latitude=" + ofToString(latitude, 4)
		+ "&longitude=" + ofToString(longitude, 4)
		+ "&current=temperature_2m%2Capparent_temperature%2Crelative_humidity_2m%2Cis_day%2Cprecipitation%2Cweather_code%2Cwind_speed_10m"
		+ "&hourly=temperature_2m&daily=weather_code%2Ctemperature_2m_max%2Ctemperature_2m_min%2Cuv_index_max%2Cprecipitation_probability_max"
		+ "&forecast_days=5&timezone=auto";
	weatherRequestId = ofLoadURLAsync(url, "weather-forecast");
}

//--------------------------------------------------------------
void ofApp::drawWeatherIcon(float x, float y, float size, int code, bool daytime){
	const float scale = size / 64.0f;
	const bool sunny = code == 0 || code == 1;
	const bool cloudy = code >= 2 && code <= 48;
	const bool rainy = code >= 51;
	const float sunX = x + (cloudy || rainy ? size * 0.18f : 0);
	const float sunY = y - size * 0.12f;
	if (sunny || cloudy) {
		ofSetColor(daytime ? orange : lime);
		if (sunny) {
			for (int i = 0; i < 8; ++i) {
				const float angle = TWO_PI * i / 8.0f;
				const float inner = size * 0.22f;
				const float outer = size * 0.32f;
				ofDrawLine(sunX + std::cos(angle) * inner, sunY + std::sin(angle) * inner,
					sunX + std::cos(angle) * outer, sunY + std::sin(angle) * outer);
			}
		}
		ofDrawCircle(sunX, sunY, size * (sunny ? 0.2f : 0.16f));
	}
	if (cloudy || rainy) {
		const ofColor cloudColor = pale;
		ofSetColor(cloudColor);
		ofDrawCircle(x - size * 0.14f, y + size * 0.04f, size * 0.19f);
		ofDrawCircle(x + size * 0.06f, y - size * 0.02f, size * 0.25f);
		ofDrawCircle(x + size * 0.27f, y + size * 0.06f, size * 0.17f);
		ofDrawRectangle(x - size * 0.29f, y + size * 0.03f, size * 0.58f, size * 0.17f);
		if (rainy) {
			ofSetColor(ofColor(131, 192, 185));
			for (int i = -1; i <= 1; ++i) {
				ofDrawLine(x + i * size * 0.17f, y + size * 0.26f,
					x + i * size * 0.17f - size * 0.04f, y + size * 0.39f);
			}
		}
	}
	if (!sunny && !cloudy && !rainy) {
		ofSetColor(lime);
		ofDrawCircle(x, y, size * 0.16f);
	}
	(void)scale;
}

//--------------------------------------------------------------
void ofApp::drawText(const std::string & text, float x, float y, int size, ofColor color){
	ofSetColor(color);
	if (fontsReady) {
		if (size >= 48) {
			displayFont.drawString(text, x, y);
		} else if (size >= 14) {
			textFont.drawString(text, x, y);
		} else {
			ofPushMatrix();
			ofTranslate(x, y - 10);
		ofScale(0.86f, 0.86f);
		ofDrawBitmapString(text, 0, 10);
		ofPopMatrix();
		}
	} else {
		const float scale = size >= 48 ? 3.2f : size >= 20 ? 1.45f : size >= 14 ? 1.0f : 0.82f;
		ofPushMatrix();
		ofTranslate(x, y - 10 * scale);
		ofScale(scale, scale);
		ofDrawBitmapString(text, 0, 10);
		ofPopMatrix();
	}
}

//--------------------------------------------------------------
std::string ofApp::weatherDescription(int code) const{
	if (code == 0) return "Clear sky";
	if (code == 1) return "Mainly clear";
	if (code == 2) return "Partly cloudy";
	if (code == 3) return "Overcast";
	if (code == 45 || code == 48) return "Foggy";
	if (code >= 51 && code <= 57) return "Drizzle";
	if (code >= 61 && code <= 67) return "Rain";
	if (code >= 71 && code <= 77) return "Snow";
	if (code >= 80 && code <= 82) return "Rain showers";
	if (code >= 85 && code <= 86) return "Snow showers";
	if (code >= 95) return "Thunderstorm";
	return "Cloudy skies";
}

//--------------------------------------------------------------
std::string ofApp::encodeUrl(const std::string & value) const{
	const char hex[] = "0123456789ABCDEF";
	std::string encoded;
	for (unsigned char character : value) {
		if (std::isalnum(character) || character == '-' || character == '_' || character == '.' || character == '~') {
			encoded.push_back(static_cast<char>(character));
		} else {
			encoded.push_back('%');
			encoded.push_back(hex[character >> 4]);
			encoded.push_back(hex[character & 15]);
		}
	}
	return encoded;
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	if (key == OF_KEY_RETURN) {
		searchActive = false;
		searchCity();
	} else if (searchActive && key == OF_KEY_BACKSPACE && !cityQuery.empty()) {
		cityQuery.pop_back();
	} else if (searchActive && key >= 32 && key <= 126 && cityQuery.size() < 48) {
		cityQuery.push_back(static_cast<char>(key));
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
	if (button != OF_MOUSE_BUTTON_LEFT) return;
	const float searchX = ofGetWidth() - 56 - 316;
	if (x >= searchX && x <= searchX + 316 && y >= 27 && y <= 69) {
		if (x >= searchX + 270) {
			searchActive = false;
			searchCity();
		} else {
			searchActive = true;
			cityQuery.clear();
		}
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}