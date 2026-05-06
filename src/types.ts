/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

export enum SaturationStyle {
  TUBE = 'tube',
  TAPE = 'tape',
  TRANSFORMER = 'transformer',
  DIODE = 'diode',
  DIGITAL = 'digital',
  PUNISH = 'punish',
}

export interface BandSettings {
  id: string;
  enabled: boolean;
  frequency: number;
  drive: number;
  style: SaturationStyle;
  mix: number;
  level: number;
  transient: number; // 0-100 (New: Preservation of transients)
  dynamics: number; // -100 to 100 (Compression/Expansion)
  width: number;
  feedback: number;
}

export interface PluginState {
  inputGain: number;
  outputGain: number;
  globalMix: number;
  bands: BandSettings[];
  oversampling: boolean;
  isBypassed: boolean;
  softClip: boolean;
}

export const DEFAULT_STATE: PluginState = {
  inputGain: 0,
  outputGain: 0,
  globalMix: 100,
  oversampling: true,
  isBypassed: false,
  softClip: true,
  bands: [
    {
      id: 'band-1',
      enabled: true,
      frequency: 200,
      drive: 20,
      style: SaturationStyle.TUBE,
      mix: 100,
      level: 0,
      width: 100,
      feedback: 0,
      dynamics: 0,
    },
    {
      id: 'band-2',
      enabled: true,
      frequency: 2500,
      drive: 15,
      style: SaturationStyle.TRANSFORMER,
      mix: 100,
      level: 0,
      width: 100,
      feedback: 0,
      dynamics: 0,
    },
    {
      id: 'band-3',
      enabled: true,
      frequency: 20000,
      drive: 10,
      style: SaturationStyle.TAPE,
      mix: 100,
      level: 0,
      width: 100,
      feedback: 0,
      dynamics: 0,
    },
  ],
};
