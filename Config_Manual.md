# OpenShock Mod Configuration Documentation

The `settings.json` file configures the OpenShock mod. 
This file must follow JSON format and include the necessary fields.

-------------------------------------------------------
Supported Fields
-------------------------------------------------------

| **Field Name**      | **Type**  | **Required** | **Default Value**       | **Description**                                                |
|---------------------|-----------|--------------|-------------------------|----------------------------------------------------------------|
| shockerID           | String    | Yes          | N/A                     | Unique identifier for the shocker device.                      |
| OpenShockToken      | String    | Yes          | N/A                     | API token for accessing the OpenShock service.                 |
| minDuration         | Integer   | No           | 300                     | Minimum duration of shock (in milliseconds). Must be >= 300.   |
| maxDuration         | Integer   | No           | 30000                   | Maximum duration of shock (in milliseconds). Must be <= 30000. |
| minIntensity        | Integer   | No           | 1                       | Minimum intensity for the shock. Must be >= 1.                 |
| maxIntensity        | Integer   | No           | 100                     | Maximum intensity for the shock. Must be <= 100.               |
| customName          | String    | Yes          | N/A                     | Custom name for the shock control session.                     |
| endpointDomain      | String    | No           | api.openshock.app       | Default API endpoint domain if not specified.                  |

-------------------------------------------------------
Validation Rules
-------------------------------------------------------

1. **Duration Ranges**:
   - `minDuration` must be >= 300.
   - `maxDuration` must be <= 30000.
   - `minDuration` must not exceed `maxDuration`.

2. **Intensity Ranges**:
   - `minIntensity` must be >= 1.
   - `maxIntensity` must be <= 100.
   - `minIntensity` must not exceed `maxIntensity`.

3. **Required Fields**:
   - `shockerID`, `OpenShockToken`, and `customName` are mandatory.

4. **Endpoint Domain**:
   - If `endpointDomain` is missing or empty, defaults to `api.openshock.app`.

-------------------------------------------------------
Example Configuration File
-------------------------------------------------------
```
{
    "shockerID": "7a3e1c5b-fb7c-4b1c-8b6e-6a2e1f8b7d92",
    "OpenShockToken": "RXLOseP4PpBmE8w59JTHUFnrIEgd5hhgeGkACgvNz7vjadAbfMOiuTev824lYP0f",
    "minDuration": 500,
    "maxDuration": 10000,
    "minIntensity": 10,
    "maxIntensity": 90,
    "customName": "ShockControl",
    "endpointDomain": "api.customdomain.com"
}
```
-------------------------------------------------------
Default Behavior
-------------------------------------------------------

- If optional fields are omitted:
  - `minDuration`: Defaults to 300.
  - `maxDuration`: Defaults to 30000.
  - `minIntensity`: Defaults to 1.
  - `maxIntensity`: Defaults to 100.
  - `endpointDomain`: Defaults to `api.openshock.app`.

-------------------------------------------------------
Error Handling
-------------------------------------------------------

- Invalid configurations will cause the mod to malfunction.
- Errors are logged and displayed in-game via pop-ups.
- Required fields must not be empty.
- Ensure `endpointDomain` is valid if provided.

-------------------------------------------------------