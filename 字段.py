import json
with open('feature_columns.json') as f:
    features = json.load(f)
print(features)