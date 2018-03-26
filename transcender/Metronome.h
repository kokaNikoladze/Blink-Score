namespace metronome {

  const long brightness = 100;

struct State {
  bool on;
  int led;
  float bpm;
  unsigned long hit_millis;
};

// pwm_duties: 0 ~ 100 % for each led
// num_leds:   how many leds
void fade(State *state, unsigned long now_millis, int *pwm_duties,
          int num_leds) {
  if (!state->on) {
    return;
  }

  for (int i = 0; i < num_leds; ++i) {
    pwm_duties[i] = 0;
  }

  unsigned long since_hit_millis = now_millis - state->hit_millis;
  unsigned long duration_millis = 60000 / state->bpm;

#if 0
  if (since_hit_millis < duration_millis) {
    pwm_duties[state->led] = 100;
  }
#endif

  long duty = brightness - brightness * since_hit_millis / duration_millis;

  duty = max(0, duty);

  pwm_duties[state->led] = duty;
}

void hit(State *state, int led, float bpm, unsigned long hit_millis) {
  state->on = true;
  state->led = led;
  state->bpm = bpm;
  state->hit_millis = hit_millis;
}

}
