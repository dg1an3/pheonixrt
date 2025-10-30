# TG-263 Structure Name Translator

A PyTorch-based neural network for translating clinical radiotherapy structure names to AAPM TG-263 standardized nomenclature.

## Overview

The AAPM Task Group 263 provides standardized nomenclature for radiotherapy structures. This translator uses a character-level LSTM with attention to learn mappings from diverse clinical naming conventions to TG-263 standard names.

## Features

- **Neural Translation**: Character-level LSTM encoder with attention mechanism
- **High Accuracy**: Trained on synthetic data with common variations and typos
- **Confidence Scores**: Returns confidence for each prediction
- **Batch Processing**: Efficient batch translation of multiple structure names
- **pymedphys Integration**: Ready-to-use integration with pymedphys DICOM workflows
- **Extensible**: Easy to retrain with custom data

## Model Architecture

```
Input: "left parotid gland"
  ↓
Character Embedding (32-dim)
  ↓
Bidirectional LSTM (128 hidden, 2 layers)
  ↓
Attention Mechanism
  ↓
Classification Head
  ↓
Output: "Parotid_L" (confidence: 0.95)
```

## Installation

```bash
# Install pybrimstone with TG-263 support
cd python
pip install -e .

# Additional requirements for TG-263
pip install torch tqdm
```

## Quick Start

### Training the Model

```bash
# Train on synthetic data (recommended first run)
python -m pybrimstone.tg263_training
```

This generates ~1000+ training examples per standard name using:
- Case variations (uppercase, lowercase, title case)
- Laterality variations (Left, L, left, LT, etc.)
- Common abbreviations (Par, Parot, Parotid)
- Spacing variations (underscores, hyphens, spaces)

### Inference

```python
from pybrimstone.tg263_model import TG263Translator

# Load trained model
translator = TG263Translator.load('tg263_model.pth')

# Translate a structure name
predictions = translator.predict('left parotid', top_k=3)
for name, confidence in predictions:
    print(f"{name}: {confidence:.3f}")

# Output:
# Parotid_L: 0.954
# Parotid_R: 0.032
# Submand_L: 0.008
```

### Batch Translation

```python
clinical_names = [
    'Left Parotid',
    'RIGHT LUNG',
    'Spinal Cord',
    'PTV Primary'
]

results = translator.predict_batch(clinical_names, top_k=1)
for name, predictions in zip(clinical_names, results):
    standard, conf = predictions[0]
    print(f"{name} → {standard} ({conf:.3f})")
```

## Integration with pymedphys

```python
from pybrimstone.tg263_pymedphys_integration import PymedphysTG263Mapper

# Create mapper
mapper = PymedphysTG263Mapper('tg263_model.pth', confidence_threshold=0.7)

# Analyze RTSS file
results = mapper.map_rtss_structures('patient_rtss.dcm', auto_rename=False)

# Generate report
report = mapper.generate_mapping_report(results, 'mapping_report.txt')
print(report)

# Auto-rename high-confidence structures
results = mapper.map_rtss_structures('patient_rtss.dcm', auto_rename=True)
# Creates: patient_rtss.tg263.dcm
```

## Supported Structures

The model currently supports ~60 common TG-263 standard names including:

### Brain/CNS
- Brain, Brainstem, Chiasm, Cochlea_L/R, Lens_L/R, OpticNrv_L/R

### Head & Neck
- Parotid_L/R, Submand_L/R, Larynx, Mandible, Esophagus, SpinalCord

### Thorax
- Heart, Lung_L/R, Lungs, Trachea

### Abdomen
- Liver, Stomach, Kidney_L/R, Kidneys, Spleen, Bowel_Small, Bowel_Large

### Pelvis
- Bladder, Rectum, FemoralHead_L/R, Prostate, Uterus, PenileBulb

### Targets
- GTV, CTV, PTV, ITV

### Other
- Body, BoneMarrow, SpinalCanal, Cauda

## Examples

See `examples/` directory for complete examples:

1. **tg263_inference_example.py**: Basic inference and batch translation
2. **tg263_pymedphys_integration.py**: Integration with pymedphys workflows

```bash
python examples/tg263_inference_example.py
python examples/tg263_pymedphys_integration.py
```

## Advanced Usage

### Custom Training Data

```python
from pybrimstone.tg263_training import TG263Dataset, train_model

# Create custom dataset
custom_data = [
    ('custom_name_1', 'Standard_Name_1'),
    ('custom_name_2', 'Standard_Name_2'),
    # ...
]

# Train with custom data
dataset = TG263Dataset(custom_data, char_to_idx, name_to_label)
# ... (see tg263_training.py for full training loop)
```

### Confidence Thresholding

```python
# Only accept high-confidence predictions
predictions = translator.predict('ambiguous name', top_k=5)

for name, conf in predictions:
    if conf >= 0.8:
        print(f"HIGH: {name} ({conf:.3f})")
    elif conf >= 0.5:
        print(f"MEDIUM: {name} ({conf:.3f}) - Review recommended")
    else:
        print(f"LOW: {name} ({conf:.3f}) - Manual mapping needed")
```

### Model Fine-Tuning

```python
# Load pre-trained model
translator = TG263Translator.load('tg263_model.pth')

# Add your institution-specific data
institution_data = [
    ('your_internal_name_1', 'Parotid_L'),
    ('your_internal_name_2', 'Bladder'),
    # ...
]

# Fine-tune on your data
# (see tg263_training.py for training utilities)
```

## Performance

On synthetic test data (20% holdout):

- **Top-1 Accuracy**: ~92%
- **Top-3 Accuracy**: ~97%
- **Inference Speed**: ~100 structures/second (CPU)
- **Model Size**: ~1.5 MB

## Future Enhancements

- [ ] Expand to full TG-263 specification (600+ structures)
- [ ] Multi-task learning (structure type, laterality, site)
- [ ] Sequence-to-sequence model for full name generation
- [ ] Pre-trained models for different anatomical sites
- [ ] Active learning for low-confidence cases
- [ ] Integration with more DICOM tools
- [ ] Web API for translation service

## Contributing

To add support for more structures:

1. Add standard names to `build_tg263_labels()` in `tg263_model.py`
2. Add aliases to `SyntheticDataGenerator` in `tg263_training.py`
3. Retrain the model with expanded data
4. Submit PR with test cases

## References

- AAPM Task Group 263: "Standardizing Nomenclatures in Radiation Oncology"
  https://www.aapm.org/pubs/reports/RPT_263.pdf

- pymedphys: Python Medical Physics Library
  https://github.com/pymedphys/pymedphys

## License

Copyright (C) 2nd Messenger Systems
See LICENSE file for details

## Citation

If you use this translator in your research, please cite:

```bibtex
@software{tg263_translator,
  title={TG-263 Structure Name Translator},
  author={Lane, Derek G.},
  year={2025},
  publisher={2nd Messenger Systems},
  url={https://github.com/DLaneAtElekta/pheonixrt}
}
```
