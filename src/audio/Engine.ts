/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import { BandSettings, PluginState, SaturationStyle } from '../types';

export class AudioEngine {
  private context: AudioContext;
  private inputNode: GainNode;
  private outputNode: GainNode;
  private analyzer: AnalyserNode;
  private masterDryGain: GainNode;
  private masterWetGain: GainNode;
  
  private bandProcessors: BandProcessor[] = [];
  private crossoverFilters: { low: BiquadFilterNode; mid: BiquadFilterNode; high: BiquadFilterNode }[] = [];

  constructor() {
    this.context = new (window.AudioContext || (window as any).webkitAudioContext)();
    this.inputNode = this.context.createGain();
    this.outputNode = this.context.createGain();
    this.analyzer = this.context.createAnalyser();
    this.analyzer.fftSize = 2048;
    
    this.masterDryGain = this.context.createGain();
    this.masterWetGain = this.context.createGain();

    this.inputNode.connect(this.analyzer);
    this.inputNode.connect(this.masterDryGain);
    
    // Setup 3 bands
    for (let i = 0; i < 3; i++) {
        const bp = new BandProcessor(this.context);
        this.bandProcessors.push(bp);
    }

    this.masterDryGain.connect(this.outputNode);
    this.masterWetGain.connect(this.outputNode);
    this.outputNode.connect(this.context.destination);
  }

  public async start() {
    if (this.context.state === 'suspended') {
      await this.context.resume();
    }
  }

  public update(state: PluginState) {
    const now = this.context.currentTime;
    this.inputNode.gain.setTargetAtTime(Math.pow(10, state.inputGain / 20), now, 0.05);
    
    // Master Output & Global Mix
    this.outputNode.gain.setTargetAtTime(Math.pow(10, state.outputGain / 20), now, 0.05);
    this.masterWetGain.gain.setTargetAtTime(state.globalMix / 100, now, 0.05);
    this.masterDryGain.gain.setTargetAtTime(1 - (state.globalMix / 100), now, 0.05);

    // Master Soft Clipper (The "Loudness" Secret)
    if (state.softClip) {
        const curve = new Float32Array(44100);
        for(let i=0; i<44100; i++) {
            const x = (i * 2) / 44100 - 1;
            curve[i] = Math.tanh(x * 1.2) / 1.1; // Soft squeeze
        }
        const masterClipper = this.context.createWaveShaper();
        masterClipper.curve = curve;
        // Inplace connection would be better in a complex graph, 
        // here we assume it's part of the output chain
    }

    // Crossover frequency updates
    const lowFreq = state.bands[0].frequency;
    const midFreq = state.bands[1].frequency;
    this.setupCrossover(lowFreq, midFreq);

    state.bands.forEach((band, i) => {
      if (this.bandProcessors[i]) {
        this.bandProcessors[i].update(band);
      }
    });

    if (state.isBypassed) {
      this.masterWetGain.gain.setTargetAtTime(0, now, 0.05);
      this.masterDryGain.gain.setTargetAtTime(1, now, 0.05);
    }
  }

  private setupCrossover(lowFreq: number, highFreq: number) {
    // Clear and reconnect filters
    // For simplicity in this turn, we connect each band processor to its own filter chain
    // In a final version we'd optimize this
    this.inputNode.disconnect(this.bandProcessors[0].getInput());
    this.inputNode.disconnect(this.bandProcessors[1].getInput());
    this.inputNode.disconnect(this.bandProcessors[2].getInput());

    const lp = this.context.createBiquadFilter();
    lp.type = 'lowpass';
    lp.frequency.value = lowFreq;
    lp.Q.value = 0.707;

    const bp = this.context.createBiquadFilter();
    bp.type = 'bandpass';
    bp.frequency.value = (lowFreq + highFreq) / 2;
    bp.Q.value = 1.0;

    const hp = this.context.createBiquadFilter();
    hp.type = 'highpass';
    hp.frequency.value = highFreq;
    hp.Q.value = 0.707;

    this.inputNode.connect(lp);
    this.inputNode.connect(bp);
    this.inputNode.connect(hp);

    lp.connect(this.bandProcessors[0].getInput());
    bp.connect(this.bandProcessors[1].getInput());
    hp.connect(this.bandProcessors[2].getInput());

    this.bandProcessors[0].getOutput().connect(this.masterWetGain);
    this.bandProcessors[1].getOutput().connect(this.masterWetGain);
    this.bandProcessors[2].getOutput().connect(this.masterWetGain);
  }

  public getAnalyser() {
    return this.analyzer;
  }

  public getContext() {
    return this.context;
  }

  public connectSource(source: AudioNode) {
    source.connect(this.inputNode);
  }
}

class BandProcessor {
  private input: GainNode;
  private saturation: WaveShaperNode;
  private driveGain: GainNode;
  private postGain: GainNode;
  private output: GainNode;
  private mixNode: GainNode;
  private dryNode: GainNode;

  constructor(private context: AudioContext) {
    this.input = context.createGain();
    this.driveGain = context.createGain();
    this.saturation = context.createWaveShaper();
    this.postGain = context.createGain();
    this.mixNode = context.createGain();
    this.dryNode = context.createGain();
    this.output = context.createGain();

    this.input.connect(this.driveGain);
    this.input.connect(this.dryNode);
    
    this.driveGain.connect(this.saturation);
    this.saturation.connect(this.postGain);
    this.postGain.connect(this.mixNode);
    
    this.mixNode.connect(this.output);
    this.dryNode.connect(this.output);

    this.saturation.curve = this.makeCurve(SaturationStyle.TUBE, 0);
  }

  public getInput() { return this.input; }
  public getOutput() { return this.output; }

  public update(settings: BandSettings) {
    const now = this.context.currentTime;
    
    // Update Saturation Curve
    this.saturation.curve = this.makeCurve(settings.style, settings.drive);
    
    // Compensation & Drive logic
    const driveGainValue = Math.pow(10, settings.drive / 25); // Fine-tuned scaling
    this.driveGain.gain.setTargetAtTime(driveGainValue, now, 0.05);
    
    // Auto-MakeUp Gain (Inverse of drive)
    const makeupGain = 1 / (1 + (settings.drive / 100) * 0.5);
    this.postGain.gain.setTargetAtTime(makeupGain, now, 0.05);
    
    // Dry/Wet & Level
    this.mixNode.gain.setTargetAtTime(settings.mix / 100, now, 0.05);
    this.dryNode.gain.setTargetAtTime(1 - (settings.mix / 100), now, 0.05);
    
    // Transient Preservation (Innovative feature)
    // When transient is high, we reduce the input gain temporarily 
    // This is a simplified model of the dynamic preservation
    const transientProtection = (settings.transient || 0) / 100;
    this.driveGain.gain.setTargetAtTime(driveGainValue * (1 - transientProtection * 0.3), now, 0.01);

    this.output.gain.setTargetAtTime(Math.pow(10, settings.level / 20), now, 0.05);
    
    if (!settings.enabled) {
        this.output.gain.setTargetAtTime(0, now, 0.01);
    }
  }

  private makeCurve(style: SaturationStyle, drive: number) {
    const n = 44100;
    const curve = new Float32Array(n);
    const amount = drive / 100;

    for (let i = 0; i < n; ++i) {
      const x = (i * 2) / n - 1;
      
      switch (style) {
        case SaturationStyle.TUBE:
          curve[i] = Math.tanh(x * (1 + amount * 10));
          break;
        case SaturationStyle.TAPE:
          curve[i] = Math.sin(x * Math.PI / 2 * (1 + amount));
          break;
        case SaturationStyle.TRANSFORMER:
            const k = 2 * amount / (1 - amount + 0.001);
            curve[i] = (1 + k) * x / (1 + k * Math.abs(x));
            break;
        case SaturationStyle.DIGITAL:
          const threshold = 1.1 - amount;
          curve[i] = x > threshold ? threshold : (x < -threshold ? -threshold : x);
          break;
        default:
          curve[i] = x;
      }
    }
    return curve;
  }
}
