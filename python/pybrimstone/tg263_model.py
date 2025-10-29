"""
TG-263 Structure Name Translator - PyTorch Implementation

A neural network model for translating clinical structure names to
AAPM TG-263 standardized nomenclature.

Copyright (C) 2nd Messenger Systems
"""

import torch
import torch.nn as nn
import torch.nn.functional as F
from typing import List, Dict, Tuple, Optional
import json
from pathlib import Path


class CharacterEmbedding(nn.Module):
    """Character-level embedding for structure names"""

    def __init__(self, vocab_size: int = 128, embedding_dim: int = 32):
        super().__init__()
        self.embedding = nn.Embedding(vocab_size, embedding_dim, padding_idx=0)
        self.vocab_size = vocab_size
        self.embedding_dim = embedding_dim

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        """
        Args:
            x: Character indices [batch, seq_len]
        Returns:
            Embedded characters [batch, seq_len, embedding_dim]
        """
        return self.embedding(x)


class StructureNameEncoder(nn.Module):
    """Bidirectional LSTM encoder for structure names"""

    def __init__(
        self,
        embedding_dim: int = 32,
        hidden_dim: int = 128,
        num_layers: int = 2,
        dropout: float = 0.2
    ):
        super().__init__()

        self.lstm = nn.LSTM(
            embedding_dim,
            hidden_dim,
            num_layers=num_layers,
            batch_first=True,
            bidirectional=True,
            dropout=dropout if num_layers > 1 else 0
        )

        self.hidden_dim = hidden_dim
        self.num_layers = num_layers

    def forward(
        self,
        x: torch.Tensor,
        lengths: Optional[torch.Tensor] = None
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Args:
            x: Embedded characters [batch, seq_len, embedding_dim]
            lengths: Actual lengths of sequences [batch]
        Returns:
            outputs: All hidden states [batch, seq_len, hidden_dim*2]
            final_hidden: Final hidden state [batch, hidden_dim*2]
        """
        if lengths is not None:
            # Pack padded sequence for efficiency
            x_packed = nn.utils.rnn.pack_padded_sequence(
                x, lengths.cpu(), batch_first=True, enforce_sorted=False
            )
            outputs_packed, (hidden, cell) = self.lstm(x_packed)
            outputs, _ = nn.utils.rnn.pad_packed_sequence(
                outputs_packed, batch_first=True
            )
        else:
            outputs, (hidden, cell) = self.lstm(x)

        # Concatenate final forward and backward hidden states
        # hidden: [num_layers*2, batch, hidden_dim]
        final_hidden = torch.cat([hidden[-2], hidden[-1]], dim=1)

        return outputs, final_hidden


class AttentionLayer(nn.Module):
    """Attention mechanism for focusing on important characters"""

    def __init__(self, hidden_dim: int):
        super().__init__()
        self.attention = nn.Linear(hidden_dim * 2, 1)

    def forward(
        self,
        encoder_outputs: torch.Tensor,
        mask: Optional[torch.Tensor] = None
    ) -> Tuple[torch.Tensor, torch.Tensor]:
        """
        Args:
            encoder_outputs: [batch, seq_len, hidden_dim*2]
            mask: [batch, seq_len] - True for valid positions
        Returns:
            context: Weighted sum of outputs [batch, hidden_dim*2]
            attention_weights: [batch, seq_len]
        """
        # Calculate attention scores
        scores = self.attention(encoder_outputs).squeeze(-1)  # [batch, seq_len]

        # Apply mask if provided
        if mask is not None:
            scores = scores.masked_fill(~mask, float('-inf'))

        # Softmax to get weights
        attention_weights = F.softmax(scores, dim=1)  # [batch, seq_len]

        # Weighted sum of encoder outputs
        context = torch.bmm(
            attention_weights.unsqueeze(1),
            encoder_outputs
        ).squeeze(1)  # [batch, hidden_dim*2]

        return context, attention_weights


class TG263Classifier(nn.Module):
    """
    Complete TG-263 structure name classifier

    Architecture:
        1. Character embedding
        2. Bidirectional LSTM encoder
        3. Attention mechanism
        4. Classification head for TG-263 standard names
    """

    def __init__(
        self,
        num_classes: int,
        vocab_size: int = 128,
        embedding_dim: int = 32,
        hidden_dim: int = 128,
        num_layers: int = 2,
        dropout: float = 0.3,
        use_attention: bool = True
    ):
        super().__init__()

        self.char_embedding = CharacterEmbedding(vocab_size, embedding_dim)
        self.encoder = StructureNameEncoder(
            embedding_dim, hidden_dim, num_layers, dropout
        )

        self.use_attention = use_attention
        if use_attention:
            self.attention = AttentionLayer(hidden_dim)
            classifier_input_dim = hidden_dim * 2
        else:
            classifier_input_dim = hidden_dim * 2

        # Classification head
        self.classifier = nn.Sequential(
            nn.Linear(classifier_input_dim, hidden_dim),
            nn.ReLU(),
            nn.Dropout(dropout),
            nn.Linear(hidden_dim, num_classes)
        )

        self.num_classes = num_classes

    def forward(
        self,
        char_indices: torch.Tensor,
        lengths: Optional[torch.Tensor] = None
    ) -> Dict[str, torch.Tensor]:
        """
        Args:
            char_indices: Character indices [batch, seq_len]
            lengths: Actual lengths [batch]
        Returns:
            Dictionary containing:
                - logits: Class logits [batch, num_classes]
                - attention_weights: Attention weights [batch, seq_len] (if use_attention)
        """
        # Embed characters
        embedded = self.char_embedding(char_indices)  # [batch, seq_len, emb_dim]

        # Encode with LSTM
        encoder_outputs, final_hidden = self.encoder(embedded, lengths)

        # Apply attention or use final hidden state
        if self.use_attention:
            # Create mask from lengths
            if lengths is not None:
                batch_size, seq_len = char_indices.shape
                mask = torch.arange(seq_len, device=char_indices.device)[None, :] < lengths[:, None]
            else:
                mask = None

            context, attention_weights = self.attention(encoder_outputs, mask)
            classifier_input = context
        else:
            classifier_input = final_hidden
            attention_weights = None

        # Classify
        logits = self.classifier(classifier_input)  # [batch, num_classes]

        return {
            'logits': logits,
            'attention_weights': attention_weights
        }


class TG263Translator:
    """
    High-level interface for TG-263 structure name translation
    """

    def __init__(
        self,
        model: TG263Classifier,
        label_to_name: Dict[int, str],
        name_to_label: Dict[str, int],
        char_to_idx: Dict[str, int],
        idx_to_char: Dict[int, str],
        device: str = 'cpu'
    ):
        self.model = model.to(device)
        self.device = device
        self.label_to_name = label_to_name
        self.name_to_label = name_to_label
        self.char_to_idx = char_to_idx
        self.idx_to_char = idx_to_char
        self.max_length = 64  # Maximum character length

    def encode_string(self, text: str) -> Tuple[torch.Tensor, int]:
        """
        Convert string to character indices

        Args:
            text: Input string
        Returns:
            char_indices: [max_length]
            actual_length: Actual length before padding
        """
        text = text.lower()[:self.max_length]

        indices = []
        for char in text:
            idx = self.char_to_idx.get(char, self.char_to_idx.get('<UNK>', 1))
            indices.append(idx)

        actual_length = len(indices)

        # Pad to max_length
        while len(indices) < self.max_length:
            indices.append(0)  # Padding index

        return torch.tensor(indices, dtype=torch.long), actual_length

    def predict(
        self,
        structure_name: str,
        top_k: int = 5,
        return_confidence: bool = True
    ) -> List[Tuple[str, float]]:
        """
        Predict TG-263 standard name for a clinical structure name

        Args:
            structure_name: Input clinical structure name
            top_k: Number of top predictions to return
            return_confidence: Include confidence scores

        Returns:
            List of (standard_name, confidence) tuples sorted by confidence
        """
        self.model.eval()

        with torch.no_grad():
            # Encode input
            char_indices, length = self.encode_string(structure_name)
            char_indices = char_indices.unsqueeze(0).to(self.device)  # [1, seq_len]
            lengths = torch.tensor([length], device=self.device)

            # Forward pass
            outputs = self.model(char_indices, lengths)
            logits = outputs['logits'][0]  # [num_classes]

            # Get probabilities
            probs = F.softmax(logits, dim=0)

            # Get top-k predictions
            top_probs, top_indices = torch.topk(probs, min(top_k, len(probs)))

            results = []
            for prob, idx in zip(top_probs.cpu(), top_indices.cpu()):
                label = idx.item()
                standard_name = self.label_to_name.get(label, f"UNKNOWN_{label}")
                confidence = prob.item()
                results.append((standard_name, confidence))

        return results

    def predict_batch(
        self,
        structure_names: List[str],
        top_k: int = 1
    ) -> List[List[Tuple[str, float]]]:
        """
        Predict TG-263 names for a batch of structure names

        Args:
            structure_names: List of clinical structure names
            top_k: Number of top predictions per input

        Returns:
            List of prediction lists
        """
        self.model.eval()

        with torch.no_grad():
            # Encode all inputs
            char_indices_list = []
            lengths_list = []

            for name in structure_names:
                char_indices, length = self.encode_string(name)
                char_indices_list.append(char_indices)
                lengths_list.append(length)

            # Stack into batch
            char_indices_batch = torch.stack(char_indices_list).to(self.device)
            lengths_batch = torch.tensor(lengths_list, device=self.device)

            # Forward pass
            outputs = self.model(char_indices_batch, lengths_batch)
            logits = outputs['logits']  # [batch, num_classes]

            # Get probabilities
            probs = F.softmax(logits, dim=1)

            # Get top-k for each input
            all_results = []
            for i in range(len(structure_names)):
                top_probs, top_indices = torch.topk(probs[i], min(top_k, probs.size(1)))

                results = []
                for prob, idx in zip(top_probs.cpu(), top_indices.cpu()):
                    label = idx.item()
                    standard_name = self.label_to_name.get(label, f"UNKNOWN_{label}")
                    confidence = prob.item()
                    results.append((standard_name, confidence))

                all_results.append(results)

        return all_results

    def save(self, path: str):
        """Save model and vocabulary"""
        save_dict = {
            'model_state_dict': self.model.state_dict(),
            'model_config': {
                'num_classes': self.model.num_classes,
                'vocab_size': self.model.char_embedding.vocab_size,
                'embedding_dim': self.model.char_embedding.embedding_dim,
                'hidden_dim': self.model.encoder.hidden_dim,
                'num_layers': self.model.encoder.num_layers,
                'use_attention': self.model.use_attention,
            },
            'label_to_name': self.label_to_name,
            'name_to_label': self.name_to_label,
            'char_to_idx': self.char_to_idx,
            'idx_to_char': self.idx_to_char,
        }
        torch.save(save_dict, path)

    @classmethod
    def load(cls, path: str, device: str = 'cpu'):
        """Load model and vocabulary"""
        checkpoint = torch.load(path, map_location=device)

        # Reconstruct model
        model = TG263Classifier(**checkpoint['model_config'])
        model.load_state_dict(checkpoint['model_state_dict'])

        # Create translator
        translator = cls(
            model=model,
            label_to_name=checkpoint['label_to_name'],
            name_to_label=checkpoint['name_to_label'],
            char_to_idx=checkpoint['char_to_idx'],
            idx_to_char=checkpoint['idx_to_char'],
            device=device
        )

        return translator


def create_vocabulary() -> Tuple[Dict[str, int], Dict[int, str]]:
    """
    Create character vocabulary for structure names

    Returns:
        char_to_idx: Character to index mapping
        idx_to_char: Index to character mapping
    """
    # Basic ASCII printable characters + special tokens
    chars = ['<PAD>', '<UNK>', '<START>', '<END>']

    # Add lowercase letters
    chars.extend([chr(i) for i in range(ord('a'), ord('z') + 1)])

    # Add digits
    chars.extend([chr(i) for i in range(ord('0'), ord('9') + 1)])

    # Add common punctuation/symbols
    chars.extend([' ', '_', '-', '(', ')', '[', ']', '.', ',', '/'])

    char_to_idx = {char: idx for idx, char in enumerate(chars)}
    idx_to_char = {idx: char for char, idx in char_to_idx.items()}

    return char_to_idx, idx_to_char


def build_tg263_labels() -> Tuple[Dict[str, int], Dict[int, str]]:
    """
    Build label mappings for TG-263 standard names

    Returns:
        name_to_label: Standard name to label index
        label_to_name: Label index to standard name
    """
    # This is a subset - in practice, you'd load the full TG-263 specification
    standard_names = [
        # Brain/CNS
        'Brain', 'Brainstem', 'Chiasm', 'Cochlea_L', 'Cochlea_R',
        'Lens_L', 'Lens_R', 'OpticNrv_L', 'OpticNrv_R',

        # Head & Neck
        'Parotid_L', 'Parotid_R', 'Submand_L', 'Submand_R',
        'SpinalCord', 'Esophagus', 'Larynx', 'Mandible',
        'BrachialPlex_L', 'BrachialPlex_R',

        # Thorax
        'Heart', 'Lung_L', 'Lung_R', 'Lungs', 'Trachea',

        # Abdomen
        'Liver', 'Stomach', 'Kidney_L', 'Kidney_R', 'Kidneys',
        'Spleen', 'Bowel_Small', 'Bowel_Large',

        # Pelvis
        'Bladder', 'Rectum', 'FemoralHead_L', 'FemoralHead_R',
        'Prostate', 'Uterus', 'PenileBulb',

        # Spine
        'SpinalCanal', 'Cauda',

        # Targets
        'GTV', 'CTV', 'PTV', 'ITV',

        # Other
        'Body', 'BoneMarrow',
    ]

    name_to_label = {name: idx for idx, name in enumerate(standard_names)}
    label_to_name = {idx: name for name, idx in name_to_label.items()}

    return name_to_label, label_to_name
