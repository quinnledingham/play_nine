inline float64
get_seconds_elapsed(App_Time *time, s64 start, s64 end) {
  float64 result = ((float64)(end - start) / (float64)time->performance_frequency);
  return result;
}

// Assets

