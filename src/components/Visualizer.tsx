/**
 * @license
 * SPDX-License-Identifier: Apache-2.0
 */

import React, { useEffect, useRef } from 'react';

interface VisualizerProps {
  analyser: AnalyserNode | null;
  className?: string;
  bands?: { frequency: number }[];
}

export const Visualizer: React.FC<VisualizerProps> = ({ analyser, className = '', bands = [] }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    if (!analyser || !canvasRef.current) return;

    const canvas = canvasRef.current;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    const bufferLength = analyser.frequencyBinCount;
    const dataArray = new Uint8Array(bufferLength);

    let animationId: number;

    const draw = () => {
      animationId = requestAnimationFrame(draw);
      analyser.getByteFrequencyData(dataArray);

      // Smooth clear
      ctx.fillStyle = 'rgba(21, 22, 25, 0.3)';
      ctx.fillRect(0, 0, canvas.width, canvas.height);

      const barWidth = (canvas.width / bufferLength) * 2.5;
      let x = 0;

      // Draw Grid
      ctx.strokeStyle = 'rgba(255, 255, 255, 0.05)';
      ctx.lineWidth = 1;
      for (let i = 1; i < 10; i++) {
        const gx = (canvas.width / 10) * i;
        ctx.beginPath();
        ctx.moveTo(gx, 0);
        ctx.lineTo(gx, canvas.height);
        ctx.stroke();
      }

      // Draw Spectrum
      ctx.beginPath();
      ctx.moveTo(0, canvas.height);
      
      for (let i = 0; i < bufferLength; i++) {
        const barHeight = (dataArray[i] / 255) * canvas.height;
        
        // Gradient coloring
        const r = dataArray[i] + (25 * (i / bufferLength));
        const g = 150 * (i / bufferLength);
        const b = 250;
        
        const y = canvas.height - barHeight;
        ctx.lineTo(x, y);

        x += barWidth + 1;
      }
      
      ctx.lineTo(canvas.width, canvas.height);
      ctx.strokeStyle = '#60a5fa';
      ctx.lineWidth = 2;
      ctx.stroke();

      // Draw Band Dividers
      bands.forEach(band => {
          const fx = (Math.log10(band.frequency) - 1.3) / (4.3 - 1.3) * canvas.width;
          ctx.setLineDash([5, 5]);
          ctx.strokeStyle = 'rgba(255,255,255,0.2)';
          ctx.beginPath();
          ctx.moveTo(fx, 0);
          ctx.lineTo(fx, canvas.height);
          ctx.stroke();
          ctx.setLineDash([]);
      });
    };

    draw();

    return () => cancelAnimationFrame(animationId);
  }, [analyser, bands]);

  return (
    <canvas 
      ref={canvasRef} 
      className={`w-full h-48 bg-[#151619] rounded-lg border border-zinc-800 ${className}`}
      width={1000}
      height={300}
    />
  );
};
