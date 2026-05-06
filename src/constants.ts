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
        { id: 'band-1', enabled: true, frequency: 250, drive: 5, style: SaturationStyle.TUBE, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0 },
        { id: 'band-2', enabled: true, frequency: 4000, drive: 10, style: SaturationStyle.TRANSFORMER, mix: 100, level: 0, width: 100, feedback: 0, dynamics: 0 },
        { id: 'band-3', enabled: true, frequency: 20000, drive: 8, style: SaturationStyle.TAPE, mix: 100, level: 0.5, width: 105, feedback: 0, dynamics: 0 },
      ],
    },
  },
};
