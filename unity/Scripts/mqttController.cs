using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using TMPro;
using UnityEngine.UI;
using XCharts;
using XCharts.Runtime;
using UnityEngine.Networking;

public class MqttWeatherController : MonoBehaviour
{
    [Tooltip("Optional name for the controller")]
    public string nameController = "Weather Controller";
    public string tag_mqttManager = ""; // To be set on the Inspector panel. It must match one of the mqttManager.cs GameObject

    
    [Tooltip("The topic to subscribe must contain this value. !!Case Sensitive!!")]
    public string topicSubscribed = ""; // The topic to subscribe, it needs to match a topic from the mqttManager

    public mqttManager _eventSender;

    
    public TextMeshProUGUI timeText;
    public GameObject objectToControl; // Pointer of the gauge or other 3D models

    // Forecast Charts
    public LineChart forecastAppTempChart;
    public LineChart forecastTempChart;
    public LineChart forecastHumdtyChart;
    public LineChart forecastWSChart;

    // Current Value Charts
    public LineChart apparentTemperaturelineChart;
    public RingChart temperatureRingChart;
    public RingChart humidityRingChart;
    public RingChart windSpeedRingChart;
    public RingChart apparentTemperatureChart;

    public State rotationAxis;
    public bool CW = true; // CW True = 1; CW False = -1
    public enum State { X, Y, Z }
    private int rotationDirection = 1;
    public float startValue = 0f;
    public float endValue = 180f;
    public float fullAngle = 180f;
    public float adjustedStart = 0f;

    private float pointerValue = 0f;
    private List<float> apparentTemperatureData = new List<float>(); // Store the last 6 apparent temperature data points
    private List<float> forecastApparentTemperatureData = new List<float>(); // Store forecast 6 temperature data points
    private List<float> Temp = new List<float>(); // Store forecast 6 temperature data points
    private List<float> Hmdty = new List<float>(); // Store forecast 6 humidity data points
    private List<float> WS = new List<float>(); // Store forecast 6 windspeed data points

    private string forecastApiUrl = "https://api.open-meteo.com/v1/forecast?latitude=51.5387&longitude=-0.0114&hourly=temperature_2m,relative_humidity_2m,wind_speed_10m&forecast_days=3";

    void Awake()
    {
        if (GameObject.FindGameObjectsWithTag(tag_mqttManager).Length > 0)
        {
            _eventSender = GameObject.FindGameObjectsWithTag(tag_mqttManager)[0].gameObject.GetComponent<mqttManager>();
            if (!_eventSender.isConnected)
            {
                _eventSender.Connect(); // Connect the Manager when the object is spawned
            }
        }
        else
        {
            Debug.LogError("At least one GameObject with mqttManager component and Tag == tag_mqttManager needs to be provided");
        }

        StartCoroutine(GetWeatherForecastData());
    }

    void OnEnable()
    {
        _eventSender.OnMessageArrived += OnMessageArrivedHandler;
    }

    void OnDisable()
    {
        _eventSender.OnMessageArrived -= OnMessageArrivedHandler;
    }

    private void OnMessageArrivedHandler(mqttObj mqttObject)
    {
        temperatureRingChart.series[0].label.formatter = "{c:f0}°C";
        humidityRingChart.series[0].label.formatter = "{c:f0}%";
        windSpeedRingChart.series[0].label.formatter = "{c:f0}kph";

        if (mqttObject.topic.Contains(topicSubscribed))
        {
            var response = JsonUtility.FromJson<WeatherData.Root>(mqttObject.msg);
            if (response != null)
            {

                // Calculate apparent temperature
                float waterVaporPressure = (response.outHumidity / 100) * 6.105f * Mathf.Exp((17.27f * response.outTemp_C) / (237.7f + response.outTemp_C));
                float apparentTemperature = (1.04f * response.outTemp_C) + (0.2f * waterVaporPressure) - (0.65f * response.windSpeed10_kph) - 2.7f;

                List<double> values1 = new List<double>
                {
                    response.outTemp_C,
                    50
                };
                List<double> values2 = new List<double>
                {
                    response.outHumidity,
                    100
                };
                List<double> values3 = new List<double>
                {
                    response.windSpeed10_kph,
                    100
                };

                temperatureRingChart.UpdateData(0, 0, values1);
                humidityRingChart.UpdateData(0, 0, values2);
                windSpeedRingChart.UpdateData(0, 0, values3);

                // Update UI with apparent temperature

                // Update current time
                string currentTime = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss"); // Format to show hours, minutes, and seconds
                timeText.text = $"Current Time: {currentTime}";

                // Update LineChart with the new apparent temperature data
                UpdateapparentTemperaturelineChart(apparentTemperature);
                UpdateRingChartWithNegativeValue(apparentTemperatureChart, apparentTemperature, 100, "°C");

                Debug.Log($"Mqtt Data Received: Time={currentTime}, Temp={response.outTemp_C}, Wind Speed={response.windSpeed10_kph}, Humidity={response.outHumidity}, Apparent Temp={apparentTemperature}");

                pointerValue = Mathf.Clamp((apparentTemperature + 11.5f) * 1.5f, startValue, endValue);
                Debug.Log("Apparent Temperature: " + apparentTemperature + ", Pointer Value: " + pointerValue);
            }
        }
    }

        private void Update()
    {
        float step = 1.5f * Time.deltaTime;
        rotationDirection = CW ? 1 : -1;

        if (pointerValue >= startValue)
        {
            Vector3 rotationVector = new Vector3();

            if (rotationAxis == State.X)
            {
                rotationVector = new Vector3(
                    (rotationDirection * ((pointerValue - startValue) * (fullAngle / (endValue - startValue)))) - adjustedStart,
                    objectToControl.transform.localEulerAngles.y,
                    objectToControl.transform.localEulerAngles.z);
            }
            else if (rotationAxis == State.Y)
            {
                rotationVector = new Vector3(
                    objectToControl.transform.localEulerAngles.x,
                    (rotationDirection * ((pointerValue - startValue) * (fullAngle / (endValue - startValue)))) - adjustedStart,
                    objectToControl.transform.localEulerAngles.z);
            }
            else if (rotationAxis == State.Z)
            {
                rotationVector = new Vector3(
                    objectToControl.transform.localEulerAngles.x,
                    objectToControl.transform.localEulerAngles.y,
                    (rotationDirection * ((pointerValue - startValue) * (fullAngle / (endValue - startValue)))) - adjustedStart);
            }

            objectToControl.transform.localRotation = Quaternion.Lerp(
                objectToControl.transform.localRotation,
                Quaternion.Euler(rotationVector),
                step);
        }
    }

    private void UpdateRingChartWithNegativeValue(RingChart chart, float value, float maxValue, string unit)
    {
        List<double> values;
        if (value < 0)
        {
            values = new List<double> { -value, maxValue + value };
            chart.series[0].itemStyle.color = Color.red;
        }
        else
        {
            values = new List<double> { value, maxValue - value };
            chart.series[0].itemStyle.color = Color.green;
        }

        chart.UpdateData(0, 0, values);

        chart.series[0].label.formatter = $"{value:F1} {unit}";

        chart.RefreshChart();
    }

    private void UpdateapparentTemperaturelineChart(float newValue)
    {
        // Add the new value to the list
        apparentTemperatureData.Add(newValue);

        // Keep only the last 6 data points
        if (apparentTemperatureData.Count > 6)
        {
            apparentTemperatureData.RemoveAt(0);
        }

        // Clear the chart's data
        var series = apparentTemperaturelineChart.GetSerie(0); // Assuming the first series is used
        series.ClearData();

        // Add the updated data points to the chart
        for (int i = 0; i < apparentTemperatureData.Count; i++)
        {
            series.AddData(i, apparentTemperatureData[i]);
        }

        // Refresh the chart to display updated data
        apparentTemperaturelineChart.RefreshChart();
    }

private IEnumerator GetWeatherForecastData()
{
    UnityWebRequest request = UnityWebRequest.Get(forecastApiUrl);
    yield return request.SendWebRequest();

    if (request.result == UnityWebRequest.Result.Success)
    {
        // 
        var response = JsonUtility.FromJson<ForecastData.Root>(request.downloadHandler.text);

        // 
        DateTime now = DateTime.Now; // UTC time
        string formattedNow = now.ToString("yyyy-MM-ddTHH:00");

        int startIndex = response.hourly.time.FindIndex(time => time == formattedNow);

        if (startIndex == -1)
        {
            Debug.LogError("No matching time found in the API data.");
            yield break;
        }

        // 
        for (int i = startIndex; i < startIndex + 6 && i < response.hourly.time.Count; i++)
        {
            // 
            float temperature = response.hourly.temperature_2m[i];
            float humidity = response.hourly.relative_humidity_2m[i];
            float windSpeed = response.hourly.wind_speed_10m[i];

            // 
            float waterVaporPressure = (humidity / 100) * 6.105f * Mathf.Exp((17.27f * temperature) / (237.7f + temperature));
            float apparentTemperature = (1.04f * temperature) + (0.2f * waterVaporPressure) - (0.65f * windSpeed) - 2.7f;

            // 
            forecastApparentTemperatureData.Add(apparentTemperature);
            Temp.Add(temperature);
            Hmdty.Add(humidity);
            WS.Add(windSpeed);

            // Debug
            Debug.Log($"API Time: {response.hourly.time[i]}, Temp: {temperature}, Humidity: {humidity}, WindSpeed: {windSpeed}, ApparentTemp: {apparentTemperature}");
        }

        
        UpdateForecastApparentTemp(forecastApparentTemperatureData);
        UpdateForecastTemp(Temp);
        UpdateForecasthumidity(Hmdty);
        UpdateForecastWS(WS);
    }
    else
    {
        Debug.LogError($"Failed to fetch forecast data: {request.error}");
    }
}



    private void UpdateForecastApparentTemp(List<float> apparentTemperatures)
    {
        var series = forecastAppTempChart.GetSerie(0); // Assuming the first series is used
        series.ClearData();

        for (int i = 0; i < apparentTemperatures.Count; i++)
        {
            series.AddData(i, apparentTemperatures[i]);
        }

        forecastAppTempChart.RefreshChart();
    }

    private void UpdateForecastTemp(List<float> temperatures)
    {
        var series = forecastTempChart.GetSerie(0); // Assuming the first series is used
        series.ClearData();

        for (int i = 0; i < temperatures.Count; i++)
        {
            series.AddData(i, temperatures[i]);
            Debug.Log($"Adding data to chart: Index={i}, Value={temperatures[i]}");
        }
        
        forecastTempChart.RefreshChart();
    }

    private void UpdateForecasthumidity(List<float> humidities)
    {
        var series = forecastHumdtyChart.GetSerie(0); // Assuming the first series is used
        series.ClearData();

        for (int i = 0; i < humidities.Count; i++)
        {
            series.AddData(i, humidities[i]);
        }

        forecastHumdtyChart.RefreshChart();
    }

    private void UpdateForecastWS(List<float> windSpeeds)
    {
        var series = forecastWSChart.GetSerie(0); // Assuming the first series is used
        series.ClearData();

        for (int i = 0; i < windSpeeds.Count; i++)
        {
            series.AddData(i, windSpeeds[i]);
        }

        forecastWSChart.RefreshChart();
    }
}

[Serializable]
public class WeatherData
{
    [Serializable]
    public class Root
    {
        public float outTemp_C;        // Temperature in Celsius
        public float windSpeed10_kph;   // Wind speed in kilometers per hour
        public float outHumidity;     // Humidity percentage
    }
}


[System.Serializable]
public class ForecastData
{
    [System.Serializable]
    public class Root
    {
        public Hourly hourly;
    }

    [System.Serializable]
    public class Hourly
    {
        public List<string> time;
        public List<float> temperature_2m;
        public List<float> relative_humidity_2m;
        public List<float> wind_speed_10m;
    }
}

