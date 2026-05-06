/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import { motion } from 'motion/react';
import React, { useCallback, useRef, useState } from 'react';

interface KnobProps {
  label: string;
  value: number;
  min: number;
  max: number;
  step?: number;
  unit?: string;
  onChange: (value: number) => void;
  className?: string;
  size?: 'sm' | 'md' | 'lg';
}

export const Knob: React.FC<KnobProps> = ({
  label,
  value,
  min,
  max,
  step = 1,
  unit = '',
  onChange,
  className = '',
  size = 'md',
}) => {
  const [isDragging, setIsDragging] = useState(false);
  const startY = useRef(0);
  const startValue = useRef(0);

  const handleMouseDown = (e: React.MouseEvent) => {
    setIsDragging(true);
    startY.current = e.clientY;
    startValue.current = value;
    window.addEventListener('mousemove', handleMouseMove);
    window.addEventListener('mouseup', handleMouseUp);
  };

  const handleMouseMove = useCallback((e: MouseEvent) => {
    const deltaY = startY.current - e.clientY;
    const range = max - min;
    const percentChange = deltaY / 200; // 200px for full range
    const newValue = Math.min(max, Math.max(min, startValue.current + percentChange * range));
    onChange(Math.round(newValue / step) * step);
  }, [min, max, step, onChange]);

  const handleMouseUp = () => {
    setIsDragging(false);
    window.removeEventListener('mousemove', handleMouseMove);
    window.removeEventListener('mouseup', handleMouseUp);
  };

  const rotate = ((value - min) / (max - min)) * 270 - 135;

  const sizeClass = {
    sm: 'w-8 h-8',
    md: 'w-12 h-12',
    lg: 'w-20 h-20',
  }[size];

  return (
    <div className={`flex flex-col items-center gap-1 ${className}`}>
      <span className="text-[10px] uppercase tracking-wider text-zinc-500 font-medium">{label}</span>
      <div 
        className={`relative ${sizeClass} cursor-ns-resize group`}
        onMouseDown={handleMouseDown}
      >
        {/* Knob Cap */}
        <div className="absolute inset-0 rounded-full bg-zinc-800 border border-zinc-700 shadow-inner" />
        {/* Glow */}
        <div className="absolute inset-0 rounded-full bg-blue-500/10 blur-sm opacity-0 group-hover:opacity-100 transition-opacity" />
        
        {/* Indicator */}
        <motion.div 
          className="absolute inset-0 flex justify-center"
          style={{ rotate }}
        >
          <div className="w-1 h-3 mt-1 bg-blue-400 rounded-full shadow-[0_0_8px_rgba(96,165,250,0.5)]" />
        </motion.div>
      </div>
      <span className="text-[10px] font-mono text-zinc-400">
        {value.toFixed(1)}{unit}
      </span>
    </div>
  );
};
