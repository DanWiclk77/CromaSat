/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import { GoogleGenAI } from "@google/genai";
import { Activity, Beaker, ChevronDown, Layers, Mic, Music, Play, Power, RotateCcw, Save, Settings2, Trash2 } from 'lucide-react';
import { motion } from 'motion/react';
import { useEffect, useRef, useState } from 'react';
import { AudioEngine } from './audio/Engine';
import { BandStrip } from './components/BandStrip';
import { Knob } from './components/Knob';
import { Visualizer } from './components/Visualizer';
import { PRESETS } from './constants';
import { DEFAULT_STATE, PluginState } from './types';

export default function App() {
  const [state, setState] = useState<PluginState>(DEFAULT_STATE);
  const [isEngineStarted, setIsEngineStarted] = useState(false);
  const [isAnalyzing, setIsAnalyzing] = useState(false);
  const [aiAnalysis, setAiAnalysis] = useState<string | null>(null);
  
  const engineRef = useRef<AudioEngine | null>(null);
  const sourceRef = useRef<AudioNode | null>(null);

  useEffect(() => {
    if (!engineRef.current) {
      engineRef.current = new AudioEngine();
    }
  }, []);

  useEffect(() => {
    if (engineRef.current && isEngineStarted) {
      engineRef.current.update(state);
    }
  }, [state, isEngineStarted]);

  const handleStart = async () => {
    if (!engineRef.current) return;
    await engineRef.current.start();
    
    // Create a dummy source for preview if no mic
    const ctx = engineRef.current.getContext();
    const osc = ctx.createOscillator();
    const lfo = ctx.createOscillator();
    const lfoGain = ctx.createGain();
    
    lfo.frequency.value = 0.5;
    lfoGain.gain.value = 100;
    lfo.connect(lfoGain);
    lfoGain.connect(osc.frequency);
    
    osc.type = 'sawtooth';
    osc.frequency.value = 110;
    
    const filter = ctx.createBiquadFilter();
    filter.type = 'lowpass';
    filter.frequency.value = 400;
    osc.connect(filter);
    
    const gain = ctx.createGain();
    gain.gain.value = 0.1;
    filter.connect(gain);
    
    engineRef.current.connectSource(gain);
    osc.start();
    lfo.start();
    
    sourceRef.current = gain;
    setIsEngineStarted(true);
  };

  const handleMicStart = async () => {
    if (!engineRef.current) return;
    try {
      const stream = await navigator.mediaDevices.getUserMedia({ audio: true });
      const ctx = engineRef.current.getContext();
      const source = ctx.createMediaStreamSource(stream);
      engineRef.current.connectSource(source);
      setIsEngineStarted(true);
      await engineRef.current.start();
    } catch (err) {
      console.error("Mic access denied", err);
      handleStart(); // Fallback to synth
    }
  };

  const handleSmartAnalysis = async () => {
    setIsAnalyzing(true);
    setAiAnalysis(null);
    
    try {
      const ai = new GoogleGenAI({ apiKey: process.env.GEMINI_API_KEY });
      const response = await ai.models.generateContent({
        model: "gemini-3-flash-preview",
        contents: `I have a multiband saturator with these settings: ${JSON.stringify(state)}. Can you describe the sonic character and suggest one high-end tweak to improve loudness and analog warmth? Provide the response as a short punchy title and one tip.`,
        config: {
          systemInstruction: "You are a world-class audio engineer and plugin developer. Analyze the current saturator settings and provide a character profile or advice. Keep it technical and concise.",
        }
      });
      
      setAiAnalysis(response.text);
    } catch (error) {
      console.error("AI analysis failed", error);
    } finally {
      setIsAnalyzing(false);
    }
  };

  const applyPreset = (presetName: string) => {
    const preset = PRESETS[presetName];
    if (preset) {
      setState(prev => ({ ...prev, ...preset.state }));
    }
  };

  return (
    <div className="min-h-screen bg-[#0E0F11] text-zinc-300 font-sans selection:bg-blue-500/30">
      {/* Header */}
      <header className="border-bottom border-zinc-800 bg-[#151619] px-6 py-3 flex items-center justify-between sticky top-0 z-50">
        <div className="flex items-center gap-3">
          <div className="w-8 h-8 bg-blue-500 rounded-lg flex items-center justify-center shadow-[0_0_15px_rgba(59,130,246,0.5)]">
            <Layers className="text-white" size={18} />
          </div>
          <div>
            <h1 className="text-sm font-bold tracking-tight text-white flex items-center gap-2">
              CROMA SAT <span className="text-[10px] py-0.5 px-1.5 bg-zinc-800 rounded text-zinc-500 font-mono">v1.2</span>
            </h1>
            <p className="text-[10px] text-zinc-500 uppercase tracking-widest font-medium">Multiband Analog Saturator</p>
          </div>
        </div>

        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2 bg-black/40 px-3 py-1.5 rounded-full border border-zinc-800">
            <Settings2 size={12} className="text-zinc-500" />
            <select 
              className="bg-transparent text-xs font-medium focus:outline-none cursor-pointer"
              onChange={(e) => applyPreset(e.target.value)}
            >
              {Object.entries(PRESETS).map(([id, p]) => (
                <option key={id} value={id} className="bg-zinc-900">{p.name}</option>
              ))}
            </select>
            <Save size={12} className="text-zinc-500 hover:text-zinc-300 cursor-pointer" />
          </div>

          <button 
            onClick={() => setState(s => ({ ...s, isBypassed: !s.isBypassed }))}
            className={`flex items-center gap-2 px-4 py-1.5 rounded-full text-xs font-bold transition-all ${
              state.isBypassed ? 'bg-zinc-800 text-zinc-500' : 'bg-blue-600 text-white shadow-[0_0_15px_rgba(37,99,235,0.4)]'
            }`}
          >
            <Power size={14} />
            {state.isBypassed ? 'BYPASSED' : 'ACTIVE'}
          </button>
        </div>
      </header>

      <main className="max-w-7xl mx-auto p-6 grid grid-cols-1 lg:grid-cols-4 gap-6">
        {/* Left Sidebar: Global Controls */}
        <div className="lg:col-span-1 flex flex-col gap-6">
          <div className="bg-[#151619] rounded-2xl border border-zinc-800 p-6 flex flex-col gap-8 shadow-xl">
             <div className="flex flex-col items-center gap-8">
                <Knob 
                  label="INPUT GAIN" 
                  value={state.inputGain} 
                  min={-24} max={24} 
                  unit="dB"
                  onChange={(v) => setState(s => ({ ...s, inputGain: v }))} 
                  size="lg"
                />
                <div className="w-1 h-20 bg-zinc-800 rounded-full flex items-end">
                    <motion.div 
                        className="w-full bg-blue-500 rounded-full"
                        animate={{ height: `${50 + state.inputGain * 2}%` }}
                    />
                </div>
                <Knob 
                  label="OUTPUT GAIN" 
                  value={state.outputGain} 
                  min={-24} max={24} 
                  unit="dB"
                  onChange={(v) => setState(s => ({ ...s, outputGain: v }))} 
                  size="lg"
                />
             </div>

             <div className="flex flex-col gap-4 pt-6 border-t border-zinc-800">
                <div className="flex justify-between items-center text-[10px] uppercase tracking-wider font-bold text-zinc-500">
                    <span>Oversampling</span>
                    <button 
                        onClick={() => setState(s => ({ ...s, oversampling: !s.oversampling }))}
                        className={`w-10 h-5 rounded-full transition-colors relative ${state.oversampling ? 'bg-blue-600' : 'bg-zinc-800'}`}
                    >
                        <div className={`absolute top-1 w-3 h-3 bg-white rounded-full transition-all ${state.oversampling ? 'left-6' : 'left-1'}`} />
                    </button>
                </div>
                <div className="flex justify-between items-center text-[10px] uppercase tracking-wider font-bold text-zinc-500">
                    <span>Soft Clip</span>
                    <button 
                        onClick={() => setState(s => ({ ...s, softClip: !s.softClip }))}
                        className={`w-10 h-5 rounded-full transition-colors relative ${state.softClip ? 'bg-blue-600' : 'bg-zinc-800'}`}
                    >
                        <div className={`absolute top-1 w-3 h-3 bg-white rounded-full transition-all ${state.softClip ? 'left-6' : 'left-1'}`} />
                    </button>
                </div>
             </div>
          </div>

          <div className="bg-gradient-to-br from-blue-600/20 to-indigo-600/10 rounded-2xl border border-blue-500/20 p-6 shadow-xl">
             <div className="flex items-center gap-2 mb-4">
               <Beaker size={16} className="text-blue-400" />
               <h3 className="text-xs font-bold text-blue-100 italic uppercase">Neural Analyzer</h3>
             </div>
             
             {aiAnalysis ? (
               <div className="flex flex-col gap-2">
                 <div className="text-xs font-mono text-zinc-300 leading-relaxed bg-black/40 p-3 rounded-lg border border-blue-500/20">
                   {aiAnalysis}
                 </div>
                 <button 
                   onClick={() => setAiAnalysis(null)}
                   className="text-[10px] text-blue-400 font-bold self-end flex items-center gap-1 hover:text-blue-300 transition-colors"
                 >
                   <RotateCcw size={10} /> RESET ANALYSIS
                 </button>
               </div>
             ) : (
               <button 
                onClick={handleSmartAnalysis}
                disabled={isAnalyzing}
                className="w-full py-4 bg-blue-600 hover:bg-blue-500 disabled:bg-zinc-800 rounded-xl text-[10px] font-black tracking-widest text-white transition-all transform active:scale-95 flex items-center justify-center gap-2"
               >
                 {isAnalyzing ? <RotateCcw className="animate-spin" size={14} /> : <Activity size={14} />}
                 {isAnalyzing ? 'ADAPTING NEURAL NET...' : 'ANALYZE SONIC PROFILE'}
               </button>
             )}
          </div>
        </div>

        {/* Center/Main Area */}
        <div className="lg:col-span-3 flex flex-col gap-6">
          {/* Main Processing Area */}
          <div className="relative bg-black rounded-lg overflow-hidden border border-zinc-800 shadow-inner">
            <Visualizer 
              analyser={engineRef.current?.getAnalyser() || null} 
              bands={state.bands}
              className="opacity-60"
            />
            
            <div className="absolute top-4 left-6 pointer-events-none">
                <div className="text-[10px] font-black text-blue-500/50 flex items-center gap-2">
                   <div className="w-1.5 h-1.5 bg-blue-500 rounded-full animate-pulse" />
                   REAL-TIME HARMONIC ANALYSIS
                </div>
            </div>

            {!isEngineStarted && (
              <div className="absolute inset-0 bg-[#0E0F11]/95 backdrop-blur-xl flex flex-col items-center justify-center z-20">
                <div className="text-center mb-8">
                  <h2 className="text-2xl font-black text-white italic tracking-tighter mb-1">CROMA SAT CORE</h2>
                  <div className="h-1 w-12 bg-blue-500 mx-auto rounded-full" />
                  <p className="text-zinc-500 text-[10px] font-bold uppercase tracking-[0.3em] mt-4">Multiband Neural Saturation</p>
                </div>
                
                <button 
                  onClick={handleMicStart}
                  className="group relative px-12 py-5 bg-white text-black font-black text-[10px] uppercase tracking-widest rounded-sm transition-all hover:bg-blue-500 hover:text-white active:scale-95 shadow-2xl"
                >
                  <span className="relative z-10 flex items-center gap-3">
                    <Activity size={16} /> INITIALIZE PLUGIN ENGINE
                  </span>
                  <div className="absolute -inset-1 bg-white/10 blur-lg group-hover:bg-blue-500/20 transition-all opacity-0 group-hover:opacity-100" />
                </button>
                
                <div className="mt-12 flex flex-col items-center gap-2">
                   <div className="flex gap-1 text-[8px] font-mono text-zinc-700">
                      <span>VST3</span>
                      <span>•</span>
                      <span>AU</span>
                      <span>•</span>
                      <span>CLAP</span>
                   </div>
                   <p className="text-zinc-800 text-[8px] font-black uppercase">Build v1.2.0-STABLE</p>
                </div>
              </div>
            )}
          </div>

          {/* Multiband Grid */}
          <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
            {state.bands.map((band, i) => (
              <BandStrip 
                key={band.id}
                index={i}
                settings={band}
                onUpdate={(newSettings) => {
                  const newBands = [...state.bands];
                  newBands[i] = newSettings;
                  setState(s => ({ ...s, bands: newBands }));
                }}
              />
            ))}
          </div>

          {/* Bottom Toolbar */}
          <div className="bg-[#151619] rounded-2xl border border-zinc-800 p-6 flex items-center justify-between shadow-xl mt-auto">
             <div className="flex flex-col gap-1">
                <span className="text-[10px] text-zinc-500 uppercase font-black tracking-widest">Character Blend</span>
                <div className="flex items-center gap-4">
                  <Knob 
                    label="DRY / WET" 
                    value={state.globalMix} 
                    min={0} max={100} 
                    unit="%"
                    onChange={(v) => setState(s => ({ ...s, globalMix: v }))} 
                    size="md"
                    className="flex-row gap-4"
                  />
                </div>
             </div>
             
             <div className="flex items-center gap-8">
                <div className="flex flex-col items-center">
                    <span className="text-[9px] text-zinc-600 uppercase font-bold mb-2">Phase</span>
                    <button className="text-[10px] font-black px-3 py-1 bg-zinc-800 rounded border border-zinc-700 text-zinc-400 hover:text-white transition-colors">LINEAR</button>
                </div>
                <div className="flex flex-col items-center">
                    <span className="text-[9px] text-zinc-600 uppercase font-bold mb-2">Display</span>
                    <button className="text-[10px] font-black px-3 py-1 bg-zinc-800 rounded border border-zinc-700 text-zinc-400 hover:text-white transition-colors">THERMAL</button>
                </div>
             </div>
          </div>
        </div>
      </main>

      {/* Footer Info */}
      <footer className="mt-12 border-t border-zinc-800 py-6 px-12 flex justify-between items-center opacity-40">
        <div className="text-[10px] font-mono tracking-tighter">
          Coded by AI Studio Build Engine | Analogue Modeling Laboratory
        </div>
        <div className="flex gap-6">
            <span className="text-[10px] font-bold uppercase tracking-widest">Stereo Core</span>
            <span className="text-[10px] font-bold uppercase tracking-widest">32-bit Floating Pt</span>
            <span className="text-[10px] font-bold uppercase tracking-widest">LR-Crossover Ready</span>
        </div>
      </footer>
    </div>
  );
}
