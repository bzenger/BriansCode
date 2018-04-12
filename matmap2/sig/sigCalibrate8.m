function cal = sigCalibrate8(varargin)

% FUNCTION cal = sigCalibrate8(acqfilenames,[mappingfile],[cal8filename])
%
% DESCRIPTION
% This function generates a calibration file.
%
% INPUT
% acqfilename    Name of the ACQ file that contains the 1mV or 10mV dataset
% mappingfile    Name of the mapping file used to map the acq-file
% cal8filename   Name for a file in which the cals are stored
%
% OUTPUT
% cal            A vector containing the calibration values (scalars for multiplying the signal strengths).
%
% SEE ALSO
%
% EXAMPLE
% >> cal =  sigCalibrate8('10mvcal.acq','torsopluscage.mapping','1mvcal.acq','mycals.cal');
% This calibrates the signals based on two files the vectors tank and cage indicate which channels
% to take from each one. The mapping file is only specified once as it is equal for both files.
% The last argument saves the calibrations in the file mycals.cal. This input is not required, you can use
% ioWriteCal as well.


    calfiles = {};
    acqcalfiles = {};
    mappingfiles = {};
    acqfiles = {};
    potentials = [];
    channels = {};

    displaybar = 0;
    calerror = 0;
    
    for p=1:nargin,
        if ischar(varargin{p}),
	        [pn,fn,ext] = fileparts(varargin{p});
	        switch ext,
            case {'.cal','.cal8'}
                calfiles{end+1} = varargin{p};
            case {'.acqcal'}
                acqcalfiles{end+1} = varargin{p};
            case '.mapping',
                mappingfiles{end+1} = varargin{p};
            case {'.acq', '.ac2'}
                acqfiles{end+1} = varargin{p};
                fprintf(1,'CAL file: %s \n',varargin{p});
            end
            if strcmp(fn,'displaybar') == 1,
                displaybar = 1;
            end
	    end
        
    end
        
    if length(mappingfiles) > 1,
        msgError('You should only one mapping file',2);    
    end
    
    if length(calfiles) > 1,
        msgError('You should only supply one output filename',2);
        calfiles = calfiles(1);
    end
    
    if length(acqfiles) > 0,
        mappingfiles = {};
    end
    
    cal = [];
    
    if displaybar == 1,
        H = waitbar(0,'CALIBRATION PROGRESS ...');
    end
    
    for p=1:length(acqfiles),

        if length(mappingfiles) > 0,
	        D = ioReadTSdata(acqfiles{p},mappingfiles);
        else
	        D = ioReadTSdata(acqfiles{p});
        end
      
        if p==1,
	        cal = zeros(D{1}.numleads,8);
        end
            
        if (~isempty(findstr(lower(D{1}.label),'1mv')))
	        ten = 0;
	    elseif (~isempty(findstr(lower(D{1}.label),'10mv')))
	        ten = 1;
	    else
            ten = 10;    % Assume it is a 10mV and futher rely on the automated error detection    
        end
       
	
	    minvalues =  [ 0.017 0.010 0.006 0.003 0.0016 0.00092 0.00053 0.00030];
	 
        for r=1:8,
	        index = find(D{1}.gain(:,1) == r-1);
	  
            if (r-1)>=4 & ~ten,
      
	            numblocks = floor(D{1}.numframes/250);
                if numblocks == 0,
	                leadmax = max(D{1}.potvals(index,:),[],2);
	                leadmin = min(D{1}.potvals(index,:),[],2);
                else
	                g = [];  h= [];
	                for q=1:numblocks,
	                    g(:,q) = max(D{1}.potvals(index,[1:250]+(q-1)*250),[],2);
	                    h(:,q) = min(D{1}.potvals(index,[1:250]+(q-1)*250),[],2);
	                end
	                leadmax = median(g,2);
	                leadmin = median(h,2);
	            end
                
                ind = find((leadmax-leadmin)<=50);       % For smallest gain the 1mV signal should be about 150 bits
                if ~isempty(ind),
                    fprintf(1,'LEAD %d DOES NOT CONTAIN A CALIBRATION SIGNAL\n',ind);
                    leadmax(ind) = 2*sqrt(2); leadmin(ind) = 0;
                    calerror = 1;
                end
               
	            cal(index,r) = 1*2*sqrt(2)./(leadmax-leadmin);
            
                if cal(index,r) < minvalues(r),
                    cal(index,r) = 10*2*sqrt(2)./(leadmax-leadmin);  
                end
            
            end
      
             % check to see if gain between 0-3 and 10mv setting
      
            if (r-1)<=3 & ten 
	
	            numblocks = floor(D{1}.numframes/250);
	            if numblocks == 0,
	                leadmax = max(D{1}.potvals(index,:),[],2);
	                leadmin = min(D{1}.potvals(index,:),[],2);
	            else
	                g = [];  h= [];
	                for q=1:numblocks,
	                    g(:,q) = max(D{1}.potvals(index,[1:250]+(q-1)*250),[],2);
	                    h(:,q) = min(D{1}.potvals(index,[1:250]+(q-1)*250),[],2);
	                end
	                leadmax = median(g,2);
	                leadmin = median(h,2);
        	    end
	
                ind = find((leadmax-leadmin)<=50);       % For smallest gain the 1mV signal should be about 150 bits
                if ~isempty(ind),
                    fprintf(1,'LEAD %d DOES NOT CONTAIN A CALIBRATION SIGNAL\n',ind);
                    leadmax(ind) = 2*sqrt(2); leadmin(ind) = 0;
                    calerror = 1;
                end
                
                cal(index,r) = 10*2*sqrt(2)./(leadmax-leadmin);
        
            end
    
            if (r-1)<=3 & ~ten
	
	            numblocks = floor(D{1}.numframes/250);
	            if numblocks == 0,
	                leadmax = max(D{1}.potvals(index,:),[],2);
	                leadmin = min(D{1}.potvals(index,:),[],2);
	            else
	                g = [];  h= [];
	                for q=1:numblocks,
	                    g(:,q) = max(D{1}.potvals(index,[1:250]+(q-1)*250),[],2);
	                    h(:,q) = min(D{1}.potvals(index,[1:250]+(q-1)*250),[],2);
	                end
	                leadmax = median(g,2);
	                leadmin = median(h,2);
	            end

                ind = find((leadmax-leadmin)<=50);       % For smallest gain the 1mV signal should be about 150 bits
                if ~isempty(ind),
                    fprintf(1,'LEAD %d DOES NOT CONTAIN A CALIBRATION SIGNAL\n',ind);
                    leadmax(ind) = 2*sqrt(2); leadmin(ind) = 0;
                    calerror = 1;
                end
                
                t = find(cal(index,r)==0);
	            cal(index(t),r) = 2*sqrt(2)./(leadmax(t)-leadmin(t));
       
                if cal(index,r) < minvalues(r),
                    cal(index,r) = 2*sqrt(2)./(leadmax-leadmin);  
                end
        
            end
    
        end
        if displaybar == 1,
            H = waitbar(p/length(acqfiles),H);
        end
        
    end    
        
    if displaybar == 1,
        close(H);
        if calerror == 1,
            errordlg('NOT EVERY CHANNEL COULD BE CALIBRATED, NOT EVERY LEAD CONTAINS A CALIBRATION SIGNAL','SIGNAL CALIBRATION');
        end    
    end
    if length(calfiles) > 0,
      ioWriteCal8(calfiles{1},cal);
    end
    
    return             
    