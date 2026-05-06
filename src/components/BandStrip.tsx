/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import { Power, Speaker, Volume2, Zap } from 'lucide-react';
import React from 'react';
import { BandSettings, SaturationStyle } from '../types';
import { Knob } from './Knob';

interface BandStripProps {
  index: number;
  settings: BandSettings;
  onUpdate: (settings: BandSettings) => void;
}

export const BandStrip: React.FC<BandStripProps> = ({ index, settings, onUpdate }) => {
  const handleChange = (key: keyof BandSettings, value: any) => {
    onUpdate({ ...settings, [key]: value });
  };

  const STYLES = [
    { id: SaturationStyle.TUBE, label: 'TUBE', color: 'bg-orange-500' },
    { id: SaturationStyle.TAPE, label: 'TAPE', color: 'bg-yellow-500' },
    { id: SaturationStyle.TRANSFORMER, label: 'TRANS', color: 'bg-blue-500' },
    { id: SaturationStyle.DIGITAL, label: 'CRUNCH', color: 'bg-red-500' },
  ];

  const bandLabels = ['LOW', 'MID', 'HIGH'];

  return (
    <div className={`flex flex-col gap-3 p-4 rounded-xl border border-zinc-700 bg-gradient-to-b from-[#1E1F23] to-[#141518] shadow-2xl transition-all ${settings.enabled ? 'opacity-100' : 'opacity-40'} group`}>
      <div className="flex items-center justify-between border-b border-zinc-800 pb-2">
        <div className="flex items-center gap-2">
            <div className={`w-2 h-2 rounded-full ${settings.enabled ? 'bg-blue-500 shadow-[0_0_8px_#3b82f6]' : 'bg-zinc-800'}`} />
            <h3 className="text-[10px] font-black tracking-[0.2em] text-zinc-400 uppercase italic">{bandLabels[index]}</h3>
        </div>
        <div className="text-[9px] font-mono text-zinc-600">{settings.frequency}Hz</div>
      </div>

      <div className="grid grid-cols-2 gap-1.5 py-1">
        {STYLES.map((style) => (
          <button
            key={style.id}
            onClick={() => handleChange('style', style.id)}
            className={`py-1 text-[8px] font-black rounded transition-all border ${
              settings.style === style.id 
                ? `${style.color} border-white/20 text-black` 
                : 'bg-black/20 border-zinc-800 text-zinc-500 hover:text-zinc-300'
            }`}
          >
            {style.label}
          </button>
        ))}
      </div>

      <div className="flex justify-between items-center bg-black/20 p-2 rounded-lg border border-zinc-800/50">
        <Knob 
          label="DRIVE" 
          value={settings.drive} 
          min={0} max={100} 
          onChange={(v) => handleChange('drive', v)} 
          size="md"
        />
        <div className="flex flex-col gap-2">
            <Knob 
                label="MIX" 
                value={settings.mix} 
                min={0} max={100} 
                unit="%"
                onChange={(v) => handleChange('mix', v)} 
                size="sm"
            />
            <Knob 
                label="SAFE-T" 
                value={settings.transient || 0} 
                min={0} max={100} 
                unit="%"
                onChange={(v) => handleChange('transient', v)} 
                size="sm"
            />
        </div>
      </div>

      <div className="flex justify-between items-center pt-1">
        <div className="flex-1 px-2">
            <input 
                type="range" 
                min={20} 
                max={20000} 
                value={settings.frequency} 
                onChange={(e) => handleChange('frequency', parseInt(e.target.value))}
                className="w-full accent-zinc-500 h-0.5 rounded-full appearance-none bg-zinc-800 cursor-ew-resize"
            />
        </div>
        <button 
          onClick={() => handleChange('enabled', !settings.enabled)}
          className={`p-1.5 rounded-md transition-all border ${settings.enabled ? 'border-zinc-600 text-white bg-zinc-700' : 'border-zinc-800 text-zinc-700 bg-zinc-900'}`}
        >
          <Power size={11} />
        </button>
      </div>
    </div>
  );
};
