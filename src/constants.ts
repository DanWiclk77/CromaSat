/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import { PluginState, SaturationStyle } from './types';

export const PRESETS: Record<string, { name: string; state: Partial<PluginState> }> = {
  default: {
    name: 'Default Init',
    state: {},
  },
  warm_vocal: {
    name: 'Warm Silky Vocal',
    state: {
      inputGain: 4,
      bands: [
        { id: 'band-1', enabled: true, frequency: 180, drive: 10, style: SaturationStyle.TUBE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0 },
        { id: 'band-2', enabled: true, frequency: 3500, drive: 25, style: SaturationStyle.TRANSFORMER, mix: 80, level: 1.5, width: 110, feedback: 0, dynamics: 0 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 15, style: SaturationStyle.TAPE, mix: 100, level: 2, width: 130, feedback: 0, dynamics: 0 },
      ],
    },
  },
  aggressive_drums: {
    name: 'Aggressive Punchy Drums',
    state: {
      inputGain: 8,
      bands: [
        { id: 'band-1', enabled: true, frequency: 150, drive: 40, style: SaturationStyle.DIGITAL, mix: 40, level: 2, width: 100, feedback: 0, dynamics: 0 },
        { id: 'band-2', enabled: true, frequency: 5000, drive: 30, style: SaturationStyle.TUBE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 20, style: SaturationStyle.TAPE, mix: 100, level: -2, width: 100, feedback: 0, dynamics: 0 },
      ],
    },
  },
  master_glue: {
    name: 'Master Glue & Saturation',
    state: {
      inputGain: 2,
      globalMix: 30,
      bands: [
        { id: 'band-1', enabled: true, frequency: 250, drive: 5, style: SaturationStyle.TUBE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0, transient: 50 },
        { id: 'band-2', enabled: true, frequency: 4000, drive: 10, style: SaturationStyle.TRANSFORMER, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0, transient: 30 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 8, style: SaturationStyle.TAPE, mix: 100, level: 0.5, width: 105, feedback: 0, dynamics: 0, transient: 20 },
      ],
    },
  },
  dark_techno_kick: {
    name: 'Techno: Dark Industrial Kick',
    state: {
      inputGain: 6,
      bands: [
        { id: 'band-1', enabled: true, frequency: 120, drive: 60, style: SaturationStyle.DIGITAL, mix: 70, level: 2, width: 100, feedback: 0, dynamics: 0, transient: 10 },
        { id: 'band-2', enabled: true, frequency: 2500, drive: 20, style: SaturationStyle.TUBE, mix: 100, level: -2, width: 100, feedback: 0, dynamics: 0, transient: 50 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 10, style: SaturationStyle.TRANSFORMER, mix: 50, level: 1, width: 120, feedback: 0, dynamics: 0, transient: 30 },
      ],
    },
  },
  berlin_minimal: {
    name: 'Techno: Berlin Minimal Highs',
    state: {
      inputGain: 2,
      bands: [
        { id: 'band-1', enabled: false, frequency: 200, drive: 0, style: SaturationStyle.TUBE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0, transient: 50 },
        { id: 'band-2', enabled: true, frequency: 5000, drive: 15, style: SaturationStyle.TRANSFORMER, mix: 100, level: 0, width: 90, feedback: 0, dynamics: 0, transient: 60 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 35, style: SaturationStyle.TAPE, mix: 90, level: 2, width: 150, feedback: 0, dynamics: 0, transient: 40 },
      ],
    },
  },
  acid_res_scream: {
    name: 'Techno: Acid Res Scream',
    state: {
      inputGain: 10,
      bands: [
        { id: 'band-1', enabled: true, frequency: 300, drive: 20, style: SaturationStyle.TUBE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0, transient: 50 },
        { id: 'band-2', enabled: true, frequency: 4500, drive: 85, style: SaturationStyle.DIGITAL, mix: 60, level: 3, width: 100, feedback: 0, dynamics: 0, transient: 10 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 20, style: SaturationStyle.TAPE, mix: 100, level: 0, width: 110, feedback: 0, dynamics: 0, transient: 40 },
      ],
    },
  },
  uplifting_trance_lead: {
    name: 'Trance: Euphoric Lead Air',
    state: {
      inputGain: 3,
      globalMix: 85,
      bands: [
        { id: 'band-1', enabled: true, frequency: 400, drive: 5, style: SaturationStyle.TUBE, mix: 100, level: -3, width: 100, feedback: 0, dynamics: 0, transient: 50 },
        { id: 'band-2', enabled: true, frequency: 6000, drive: 25, style: SaturationStyle.TRANSFORMER, mix: 100, level: 2, width: 120, feedback: 0, dynamics: 0, transient: 30 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 40, style: SaturationStyle.TAPE, mix: 100, level: 4, width: 180, feedback: 0, dynamics: 0, transient: 20 },
      ],
    },
  },
  psytrance_bass_grit: {
    name: 'Trance: Psy-Bass Texture',
    state: {
      inputGain: 5,
      bands: [
        { id: 'band-1', enabled: true, frequency: 150, drive: 25, style: SaturationStyle.TUBE, mix: 40, level: 1, width: 100, feedback: 0, dynamics: 0, transient: 5 },
        { id: 'band-2', enabled: true, frequency: 1500, drive: 45, style: SaturationStyle.DIGITAL, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0, transient: 40 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 10, style: SaturationStyle.TRANSFORMER, mix: 100, level: -4, width: 100, feedback: 0, dynamics: 0, transient: 60 },
      ],
    },
  },
  prog_trance_pluck: {
    name: 'Trance: Prog Pluck Snap',
    state: {
      inputGain: 2,
      bands: [
        { id: 'band-1', enabled: true, frequency: 300, drive: 12, style: SaturationStyle.TUBE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0, transient: 80 },
        { id: 'band-2', enabled: true, frequency: 3500, drive: 20, style: SaturationStyle.TRANSFORMER, mix: 100, level: 1, width: 110, feedback: 0, dynamics: 0, transient: 90 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 15, style: SaturationStyle.TAPE, mix: 100, level: 1, width: 130, feedback: 0, dynamics: 0, transient: 70 },
      ],
    },
  },
  hard_techno_roar: {
    name: 'Techno: Hardcore Screamer',
    state: {
      inputGain: 12,
      bands: [
        { id: 'band-1', enabled: true, frequency: 200, drive: 40, style: SaturationStyle.DIGITAL, mix: 80, level: 5, width: 100, feedback: 0, dynamics: 0, transient: 0 },
        { id: 'band-2', enabled: true, frequency: 6000, drive: 70, style: SaturationStyle.DIGITAL, mix: 100, level: 2, width: 100, feedback: 0, dynamics: 0, transient: 5 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 30, style: SaturationStyle.TAPE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0, transient: 20 },
      ],
    },
  },
};
