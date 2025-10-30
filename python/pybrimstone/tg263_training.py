"""
Training utilities for TG-263 structure name translator

Includes synthetic data generation and training loop.

Copyright (C) 2nd Messenger Systems
"""

import torch
import torch.nn as nn
import torch.optim as optim
from torch.utils.data import Dataset, DataLoader
from typing import List, Tuple, Dict
import random
import numpy as np
from tqdm import tqdm

from .tg263_model import (
    TG263Classifier,
    TG263Translator,
    create_vocabulary,
    build_tg263_labels
)


class TG263Dataset(Dataset):
    """Dataset of structure names and their TG-263 labels"""

    def __init__(
        self,
        examples: List[Tuple[str, str]],  # (clinical_name, standard_name)
        char_to_idx: Dict[str, int],
        name_to_label: Dict[str, int],
        max_length: int = 64
    ):
        self.examples = examples
        self.char_to_idx = char_to_idx
        self.name_to_label = name_to_label
        self.max_length = max_length

    def __len__(self) -> int:
        return len(self.examples)

    def __getitem__(self, idx: int) -> Tuple[torch.Tensor, int, int]:
        clinical_name, standard_name = self.examples[idx]

        # Encode clinical name to character indices
        clinical_name = clinical_name.lower()[:self.max_length]
        char_indices = []
        for char in clinical_name:
            char_idx = self.char_to_idx.get(char, self.char_to_idx.get('<UNK>', 1))
            char_indices.append(char_idx)

        actual_length = len(char_indices)

        # Pad to max_length
        while len(char_indices) < self.max_length:
            char_indices.append(0)

        # Get label
        label = self.name_to_label.get(standard_name, -1)
        if label == -1:
            raise ValueError(f"Unknown standard name: {standard_name}")

        return (
            torch.tensor(char_indices, dtype=torch.long),
            label,
            actual_length
        )


def collate_fn(batch):
    """Custom collate function for batching"""
    char_indices, labels, lengths = zip(*batch)

    char_indices = torch.stack(char_indices)
    labels = torch.tensor(labels, dtype=torch.long)
    lengths = torch.tensor(lengths, dtype=torch.long)

    return char_indices, labels, lengths


class SyntheticDataGenerator:
    """Generate synthetic training data with common variations"""

    def __init__(self, standard_names: List[str]):
        self.standard_names = standard_names

        # Define common aliases and variations
        self.alias_patterns = {
            # Laterality variations
            '_L': ['_L', '_l', ' L', ' l', ' left', ' Left', ' lt', ' LT', '-L', '-l'],
            '_R': ['_R', '_r', ' R', ' r', ' right', ' Right', ' rt', ' RT', '-R', '-r'],

            # Common abbreviations
            'Parotid': ['Parotid', 'parotid', 'PAROTID', 'Parot', 'Par'],
            'Submand': ['Submand', 'submand', 'SUBMAND', 'Submandibular', 'submandibular'],
            'SpinalCord': ['SpinalCord', 'Spinal Cord', 'spinal cord', 'SC', 'Cord', 'cord'],
            'Brainstem': ['Brainstem', 'Brain Stem', 'brain stem', 'BS', 'brainstem'],
            'Bladder': ['Bladder', 'bladder', 'BLADDER', 'UB', 'Urinary Bladder'],
            'Rectum': ['Rectum', 'rectum', 'RECTUM', 'Rectal'],
            'Esophagus': ['Esophagus', 'esophagus', 'ESOPHAGUS', 'Eso', 'Oesophagus'],
            'FemoralHead': ['FemoralHead', 'Femoral Head', 'femoral head', 'Femur Head', 'FemHead'],
            'OpticNrv': ['OpticNrv', 'Optic Nerve', 'optic nerve', 'ON', 'OptNerve'],
            'Cochlea': ['Cochlea', 'cochlea', 'COCHLEA', 'Inner Ear'],
            'Lens': ['Lens', 'lens', 'LENS'],
            'Heart': ['Heart', 'heart', 'HEART', 'Cardiac'],
            'Lung': ['Lung', 'lung', 'LUNG'],
            'Lungs': ['Lungs', 'lungs', 'LUNGS', 'Both Lungs', 'Bilateral Lungs'],
            'Liver': ['Liver', 'liver', 'LIVER', 'Hepatic'],
            'Kidney': ['Kidney', 'kidney', 'KIDNEY'],
            'Kidneys': ['Kidneys', 'kidneys', 'KIDNEYS', 'Both Kidneys'],
            'Prostate': ['Prostate', 'prostate', 'PROSTATE', 'Prostate Gland'],

            # Targets
            'GTV': ['GTV', 'gtv', 'Gtv', 'GTV Primary', 'Primary GTV'],
            'CTV': ['CTV', 'ctv', 'Ctv', 'CTV Primary', 'Primary CTV'],
            'PTV': ['PTV', 'ptv', 'Ptv', 'PTV Primary', 'Primary PTV'],
            'ITV': ['ITV', 'itv', 'Itv', 'Internal Target'],

            # Other
            'Body': ['Body', 'body', 'BODY', 'External', 'Skin', 'Patient'],
            'BoneMarrow': ['BoneMarrow', 'Bone Marrow', 'bone marrow', 'Marrow'],
        }

        # Noise patterns to add realism
        self.noise_patterns = [
            lambda s: s,  # No noise
            lambda s: s.upper(),
            lambda s: s.lower(),
            lambda s: s.title(),
            lambda s: s + ' ',  # Trailing space
            lambda s: ' ' + s,  # Leading space
            lambda s: s.replace('_', ' '),
            lambda s: s.replace('_', '-'),
            lambda s: s.replace(' ', '_'),
        ]

    def generate_variations(self, standard_name: str, num_variations: int = 10) -> List[str]:
        """Generate variations of a standard name"""
        variations = [standard_name]  # Include the standard name itself

        # Split by known patterns
        base_name = standard_name
        for pattern, replacements in self.alias_patterns.items():
            if pattern in base_name:
                base_name_part = base_name.replace(pattern, '')

                # Generate combinations
                for repl in replacements[:min(num_variations // 2, len(replacements))]:
                    variations.append(base_name_part + repl)

        # Apply alias patterns to whole name
        for pattern, replacements in self.alias_patterns.items():
            if pattern in standard_name or pattern.replace('_', ' ') in standard_name:
                continue  # Already handled above

            # Check if base matches
            base_check = standard_name.split('_')[0] if '_' in standard_name else standard_name
            if base_check in self.alias_patterns:
                for alias in self.alias_patterns[base_check][:3]:
                    var = standard_name.replace(base_check, alias)
                    variations.append(var)

        # Apply noise patterns
        noisy_variations = []
        for var in variations[:num_variations // 2]:
            noise_fn = random.choice(self.noise_patterns)
            noisy_variations.append(noise_fn(var))

        variations.extend(noisy_variations)

        # Remove duplicates and limit
        variations = list(set(variations))
        random.shuffle(variations)

        return variations[:num_variations]

    def generate_dataset(
        self,
        num_variations_per_name: int = 15
    ) -> List[Tuple[str, str]]:
        """
        Generate full training dataset

        Returns:
            List of (clinical_name, standard_name) pairs
        """
        dataset = []

        for standard_name in self.standard_names:
            variations = self.generate_variations(standard_name, num_variations_per_name)

            for variation in variations:
                dataset.append((variation, standard_name))

        random.shuffle(dataset)
        return dataset


def train_model(
    model: TG263Classifier,
    train_loader: DataLoader,
    val_loader: DataLoader,
    num_epochs: int = 50,
    learning_rate: float = 0.001,
    device: str = 'cpu',
    patience: int = 5
) -> Dict:
    """
    Train the TG-263 classifier

    Args:
        model: TG263Classifier model
        train_loader: Training data loader
        val_loader: Validation data loader
        num_epochs: Number of epochs
        learning_rate: Learning rate
        device: Device to train on
        patience: Early stopping patience

    Returns:
        Training history dictionary
    """
    model = model.to(device)
    criterion = nn.CrossEntropyLoss()
    optimizer = optim.Adam(model.parameters(), lr=learning_rate)
    scheduler = optim.lr_scheduler.ReduceLROnPlateau(
        optimizer, mode='min', factor=0.5, patience=3, verbose=True
    )

    history = {
        'train_loss': [],
        'train_acc': [],
        'val_loss': [],
        'val_acc': [],
    }

    best_val_loss = float('inf')
    patience_counter = 0

    for epoch in range(num_epochs):
        # Training phase
        model.train()
        train_loss = 0.0
        train_correct = 0
        train_total = 0

        pbar = tqdm(train_loader, desc=f'Epoch {epoch+1}/{num_epochs} [Train]')
        for char_indices, labels, lengths in pbar:
            char_indices = char_indices.to(device)
            labels = labels.to(device)
            lengths = lengths.to(device)

            optimizer.zero_grad()

            outputs = model(char_indices, lengths)
            logits = outputs['logits']

            loss = criterion(logits, labels)
            loss.backward()
            optimizer.step()

            train_loss += loss.item()
            _, predicted = torch.max(logits, 1)
            train_correct += (predicted == labels).sum().item()
            train_total += labels.size(0)

            pbar.set_postfix({'loss': loss.item(), 'acc': train_correct / train_total})

        avg_train_loss = train_loss / len(train_loader)
        train_acc = train_correct / train_total

        # Validation phase
        model.eval()
        val_loss = 0.0
        val_correct = 0
        val_total = 0

        with torch.no_grad():
            for char_indices, labels, lengths in tqdm(val_loader, desc='[Val]'):
                char_indices = char_indices.to(device)
                labels = labels.to(device)
                lengths = lengths.to(device)

                outputs = model(char_indices, lengths)
                logits = outputs['logits']

                loss = criterion(logits, labels)
                val_loss += loss.item()

                _, predicted = torch.max(logits, 1)
                val_correct += (predicted == labels).sum().item()
                val_total += labels.size(0)

        avg_val_loss = val_loss / len(val_loader)
        val_acc = val_correct / val_total

        # Update history
        history['train_loss'].append(avg_train_loss)
        history['train_acc'].append(train_acc)
        history['val_loss'].append(avg_val_loss)
        history['val_acc'].append(val_acc)

        print(f'\nEpoch {epoch+1}:')
        print(f'  Train Loss: {avg_train_loss:.4f}, Train Acc: {train_acc:.4f}')
        print(f'  Val Loss: {avg_val_loss:.4f}, Val Acc: {val_acc:.4f}')

        # Learning rate scheduling
        scheduler.step(avg_val_loss)

        # Early stopping
        if avg_val_loss < best_val_loss:
            best_val_loss = avg_val_loss
            patience_counter = 0
        else:
            patience_counter += 1
            if patience_counter >= patience:
                print(f'\nEarly stopping triggered after epoch {epoch+1}')
                break

    return history


def train_tg263_model(
    output_path: str = 'tg263_model.pth',
    num_variations: int = 20,
    num_epochs: int = 50,
    batch_size: int = 32,
    device: str = 'cuda' if torch.cuda.is_available() else 'cpu'
):
    """
    Complete training pipeline for TG-263 model

    Args:
        output_path: Where to save the trained model
        num_variations: Number of variations per standard name
        num_epochs: Training epochs
        batch_size: Batch size
        device: Training device
    """
    print("Setting up TG-263 model training...")
    print(f"Using device: {device}")

    # Create vocabulary and labels
    char_to_idx, idx_to_char = create_vocabulary()
    name_to_label, label_to_name = build_tg263_labels()

    print(f"Vocabulary size: {len(char_to_idx)}")
    print(f"Number of classes: {len(label_to_name)}")

    # Generate synthetic data
    print("\nGenerating synthetic training data...")
    generator = SyntheticDataGenerator(list(label_to_name.values()))
    full_dataset = generator.generate_dataset(num_variations)

    print(f"Generated {len(full_dataset)} training examples")

    # Split into train/val
    val_split = 0.2
    val_size = int(len(full_dataset) * val_split)
    train_dataset = full_dataset[val_size:]
    val_dataset = full_dataset[:val_size]

    print(f"Train size: {len(train_dataset)}, Val size: {len(val_dataset)}")

    # Create datasets
    train_ds = TG263Dataset(train_dataset, char_to_idx, name_to_label)
    val_ds = TG263Dataset(val_dataset, char_to_idx, name_to_label)

    # Create data loaders
    train_loader = DataLoader(
        train_ds, batch_size=batch_size, shuffle=True, collate_fn=collate_fn
    )
    val_loader = DataLoader(
        val_ds, batch_size=batch_size, shuffle=False, collate_fn=collate_fn
    )

    # Create model
    print("\nCreating model...")
    model = TG263Classifier(
        num_classes=len(label_to_name),
        vocab_size=len(char_to_idx),
        embedding_dim=32,
        hidden_dim=128,
        num_layers=2,
        dropout=0.3,
        use_attention=True
    )

    print(f"Model parameters: {sum(p.numel() for p in model.parameters()):,}")

    # Train model
    print("\nStarting training...")
    history = train_model(
        model,
        train_loader,
        val_loader,
        num_epochs=num_epochs,
        learning_rate=0.001,
        device=device,
        patience=7
    )

    # Create translator and save
    print(f"\nSaving model to {output_path}...")
    translator = TG263Translator(
        model=model,
        label_to_name=label_to_name,
        name_to_label=name_to_label,
        char_to_idx=char_to_idx,
        idx_to_char=idx_to_char,
        device=device
    )
    translator.save(output_path)

    print("Training complete!")

    return translator, history


if __name__ == '__main__':
    # Train the model
    translator, history = train_tg263_model(
        output_path='tg263_model.pth',
        num_variations=20,
        num_epochs=50,
        batch_size=32
    )

    # Test on some examples
    print("\nTesting model...")
    test_names = [
        'left parotid',
        'Parotid_L',
        'RIGHT LUNG',
        'Spinal Cord',
        'gtv primary',
        'Bladder',
        'Lt Lens'
    ]

    for name in test_names:
        predictions = translator.predict(name, top_k=3)
        print(f"\n'{name}' →")
        for pred_name, conf in predictions:
            print(f"  {pred_name}: {conf:.3f}")
