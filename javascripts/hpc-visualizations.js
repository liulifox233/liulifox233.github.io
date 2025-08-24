// HPC Performance Visualization Components
class HPCVisualizations {
    
    
    // Performance Line Chart Component
    static createPerformanceChart(containerId, data, options = {}) {
        const container = document.getElementById(containerId);
        if (!container) return;
        
        const defaultOptions = {
            width: 800,
            height: 400,
            margin: { top: 20, right: 20, bottom: 30, left: 40 },
            colors: ['#e41a1c', '#377eb8', '#4daf4a', '#984ea3']
        };
        
        const config = { ...defaultOptions, ...options };
        
        container.innerHTML = '';
        
        const svg = d3.select(container)
            .append('svg')
            .attr('width', config.width)
            .attr('height', config.height)
            .attr('class', 'performance-chart');
        
        const g = svg.append('g');
        
        // Create scales
        const xScale = d3.scaleLinear()
            .domain([0, data.length - 1])
            .range([config.margin.left, config.width - config.margin.right]);
        
        const yScale = d3.scaleLinear()
            .domain([0, d3.max(data, d => d.value)])
            .range([config.height - config.margin.bottom, config.margin.top]);
        
        // Create line generator
        const line = d3.line()
            .x((d, i) => xScale(i))
            .y(d => yScale(d.value))
            .curve(d3.curveMonotoneX);
        
        // Draw line
        g.append('path')
            .datum(data)
            .attr('fill', 'none')
            .attr('stroke', config.colors[0])
            .attr('stroke-width', 2)
            .attr('d', line);
        
        // Draw points
        g.selectAll('circle')
            .data(data)
            .enter()
            .append('circle')
            .attr('cx', (d, i) => xScale(i))
            .attr('cy', d => yScale(d.value))
            .attr('r', 4)
            .attr('fill', config.colors[1])
            .attr('stroke', 'white')
            .attr('stroke-width', 1);
        
        // Add labels
        g.selectAll('text')
            .data(data)
            .enter()
            .append('text')
            .attr('x', (d, i) => xScale(i))
            .attr('y', d => yScale(d.value) - 10)
            .attr('text-anchor', 'middle')
            .attr('font-size', '10px')
            .attr('fill', '#666')
            .text(d => d.value + 's');
        
        // Add axes
        this.addAxes(g, xScale, yScale, config);
        
        return svg;
    }
    
    static addAxes(g, xScale, yScale, config) {
        // X axis
        g.append('g')
            .attr('transform', `translate(0, ${config.height - config.margin.bottom})`)
            .call(d3.axisBottom(xScale).ticks(5));
        
        // Y axis
        g.append('g')
            .attr('transform', `translate(${config.margin.left}, 0)`)
            .call(d3.axisLeft(yScale));
    }
    
    // Hot Code Highlighting Component
    static createHotCode(containerId, codeData, options = {}) {
        const container = document.getElementById(containerId);
        if (!container) return;
        
        const defaultOptions = {
            theme: 'github-dark',
            showHeatmap: true,
            maxLines: 50
        };
        
        const config = { ...defaultOptions, ...options };
        
        container.innerHTML = '';
        
        // Create code container
        const codeContainer = document.createElement('div');
        codeContainer.className = 'hot-code-container';
        codeContainer.style.cssText = `
            position: relative;
            background: var(--md-code-bg-color);
            border-radius: 0.1rem;
            padding: 16px;
            font-family: var(--md-code-font-family, 'Monaco', 'Menlo', 'Ubuntu Mono', monospace);
            font-size: 14px;
            line-height: 1.5;
            color: var(--md-code-fg-color);
            overflow-x: auto;
        `;
        
        // Create heatmap background (removed as it conflicts with code block styling)
        
        // Create code content
        const codeContent = document.createElement('pre');
        codeContent.style.cssText = `
            position: relative;
            z-index: 1;
            margin: 0;
            white-space: pre-wrap;
        `;
        
        // Add code lines with highlighting
        codeData.slice(0, config.maxLines).forEach((line, index) => {
            const lineDiv = document.createElement('div');
            lineDiv.style.cssText = `
                display: flex;
                align-items: center;
                min-height: 20px;
            `;
            
            // Line number
            const lineNumber = document.createElement('span');
            lineNumber.style.cssText = `
                color: var(--md-code-hl-comment-color, #6e7681);
                margin-right: 16px;
                min-width: 30px;
                text-align: right;
                user-select: none;
                opacity: 0.7;
            `;
            lineNumber.textContent = (index + 1).toString().padStart(2, ' ');
            
            // Code content
            const codeSpan = document.createElement('span');
            codeSpan.textContent = line.code;
            
            // Add heat intensity background
            if (config.showHeatmap && line.heat > 0) {
                const intensity = Math.min(1, line.heat / 100);
                const alpha = 0.1 + intensity * 0.2;
                codeSpan.style.backgroundColor = `rgba(255, 100, 100, ${alpha})`;
                codeSpan.style.padding = '0 2px';
                codeSpan.style.margin = '0 -2px';
                codeSpan.style.borderRadius = '2px';
            }
            
            lineDiv.appendChild(lineNumber);
            lineDiv.appendChild(codeSpan);
            codeContent.appendChild(lineDiv);
        });
        
        codeContainer.appendChild(codeContent);
        container.appendChild(codeContainer);
        
        return codeContainer;
    }
}

// Initialize visualizations when page loads
document.addEventListener('DOMContentLoaded', function() {
    // Auto-initialize components with data attributes
    document.querySelectorAll('[data-hpc-viz="performance"]').forEach(container => {
        const dataAttr = container.getAttribute('data-performance-data');
        if (dataAttr) {
            const decodedData = dataAttr.replace(/&quot;/g, '"').replace(/&lt;/g, '<').replace(/&gt;/g, '>');
            const data = JSON.parse(decodedData || '[]');
            HPCVisualizations.createPerformanceChart(container.id, data);
        }
    });
    
    document.querySelectorAll('[data-hpc-viz="hotcode"]').forEach(container => {
        const dataAttr = container.getAttribute('data-code-data');
        if (dataAttr) {
            const decodedData = dataAttr.replace(/&quot;/g, '"').replace(/&lt;/g, '<').replace(/&gt;/g, '>');
            const data = JSON.parse(decodedData || '[]');
            HPCVisualizations.createHotCode(container.id, data);
        }
    });
});